#include "websocket_server.h"
#include "/Users/omvibhandik/Desktop/deribit_oems/src/api/deribit_api.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "/Users/omvibhandik/Desktop/deribit_oems/src/include/json.hpp"
#include "/Users/omvibhandik/Desktop/deribit_oems/src/api/performance_utils.h"

using json = nlohmann::json;

WebSocketOrderBookServer::WebSocketOrderBookServer(const std::string &clientId, const std::string &clientSecret)
    : clientId(clientId), clientSecret(clientSecret), accessToken("")
{
    server.init_asio();

    // Set WebSocket handlers
    server.set_open_handler([this](websocketpp::connection_hdl hdl)
                            { this->onOpen(hdl); });
    server.set_close_handler([this](websocketpp::connection_hdl hdl)
                             { this->onClose(hdl); });
    server.set_message_handler([this](websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg)
                               { this->onMessage(hdl, msg); });
}

void WebSocketOrderBookServer::run(uint16_t port)
{
    server.listen(port);
    server.start_accept();
    std::cout << "WebSocket server listening on port " << port << std::endl;
    server.run();
}

void WebSocketOrderBookServer::startOrderBookUpdates(int intervalSeconds)
{

    std::thread([this, intervalSeconds]()
                {
        while (true) {
            {
                PerformanceMetrics metrics;

                startTradingLoopTiming(metrics);

                std::lock_guard<std::mutex> lock(subscriptionMutex);

                if (subscriptions.empty()) {
                    std::cout << "No symbols subscribed, skipping this cycle." << std::endl;
                } else {
                    std::cout << "Processing subscribed symbols:" << std::endl;
                    
                    for (const auto& [symbol, clients] : subscriptions) {
                        std::cout << "- " << symbol << " (" << clients.size() << " clients)" << std::endl;

                        if (!clients.empty()) {
                            std::string token = getValidAccessToken();

                            // Fetch live order book
                            json payload = {
                                {"jsonrpc", "2.0"},
                                {"method", "public/get_order_book"},
                                {"params", {{"instrument_name", symbol}}},
                                {"id", 15}
                            };
                            std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book", payload, token);

                            if (!response.empty()) {
                                try {
                                    auto parsedOrderBook = json::parse(response);
                                    if (parsedOrderBook.contains("result")) {
                                        json formattedPayload = {
                                            {"symbol", symbol},
                                            {"orderBook", parsedOrderBook["result"]}
                                        };

                                        for (const auto& hdl : clients) {
                                            server.send(hdl, formattedPayload.dump(4), websocketpp::frame::opcode::text);
                                            std::cout << "Broadcasting order book for symbol: " << symbol << std::endl;
                                        }
                                    }
                                } catch (const std::exception& e) {
                                    std::cerr << "Error parsing order book for symbol: " << symbol
                                              << " -> " << e.what() << std::endl;
                                }
                            } else {
                                std::cerr << "Failed to fetch order book for symbol: " << symbol << std::endl;
                            }
                        }
                    }
                }

                // // Broadcast aggregated updates to all connected clients
                // for (const auto& [symbol, clients] : subscriptions) {
                //     for (const auto& hdl : clients) {
                //         if (!aggregatedUpdates.empty()) {
                //             server.send(hdl, aggregatedUpdates.dump(4), websocketpp::frame::opcode::text);
                //         }
                //     }
                // }


                endTradingLoopTiming(metrics);
                metrics.measureTradingLoopLatency();
            }

            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        } })
        .detach(); // Detach the thread to run in the background
}

void WebSocketOrderBookServer::onOpen(websocketpp::connection_hdl hdl)
{
    clients.insert(hdl);
    std::cout << "Client connected" << std::endl;
}

void WebSocketOrderBookServer::onClose(websocketpp::connection_hdl hdl)
{
    clients.erase(hdl);
    std::cout << "Client disconnected" << std::endl;
}

void WebSocketOrderBookServer::onMessage(websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg)
{
    PerformanceMetrics metrics; // Create a performance metrics object

    try
    {
        auto payload = json::parse(msg->get_payload());
        std::string action = payload["action"];
        std::string token = getValidAccessToken(); // Dynamically fetch or reuse the token

        if (action == "subscribe")
        {
            std::string symbol = payload["symbol"];
            std::cout << "Client subscribed to " << symbol << std::endl;

            // Start the performance timers for order placement
            startOrderPlacementTiming(metrics);

            {
                std::lock_guard<std::mutex> lock(subscriptionMutex);
                subscriptions[symbol].insert(hdl);
                std::cout << "Symbol added to subscriptions: " << symbol << std::endl;
            }

            // Fetch live order book data and measure API response time
            json requestPayload = {
                {"jsonrpc", "2.0"},
                {"method", "public/get_order_book"},
                {"params", {{"instrument_name", symbol}}},
                {"id", 15}};

            std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book", requestPayload, token);


            if (response.empty())
            {
                json errorResponse = {{"status", "error"}, {"message", "Failed to fetch order book"}};
                server.send(hdl, errorResponse.dump(), websocketpp::frame::opcode::text);
                return;
            }

            // Ensure the response is valid JSON
            try
            {
                json responseJson = json::parse(response);

                // Check if the 'result' key exists and handle it
                if (!responseJson.contains("result"))
                {
                    std::cerr << "Error: No 'result' key in the response." << std::endl;
                    json errorResponse = {{"status", "error"}, {"message", "Invalid response format"}};
                    server.send(hdl, errorResponse.dump(), websocketpp::frame::opcode::text);
                    return;
                }

                // End the performance timers for market data fetching
                endOrderPlacementTiming(metrics);
                metrics.measureOrderPlacementLatency();

                // Start WebSocket Propagation Latency Timer
                startWebSocketTiming(metrics);

                // Send the response to WebSocket client
                server.send(hdl, response, websocketpp::frame::opcode::text);

                // End WebSocket Propagation Latency Timer after sending the message
                endWebSocketTiming(metrics);
                metrics.measureWebSocketPropagationLatency();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing market data response: " << e.what() << std::endl;
                json errorResponse = {{"status", "error"}, {"message", "Invalid JSON response"}};
                server.send(hdl, errorResponse.dump(), websocketpp::frame::opcode::text);
                return;
            }
        }
        else if (action == "unsubscribe")
        {
            std::string symbol = payload["symbol"];
            std::cout << "Client unsubscribed from " << symbol << std::endl;

            // Start measuring unsubscribe performance
            startTradingLoopTiming(metrics);

            {
                std::lock_guard<std::mutex> lock(subscriptionMutex);
                subscriptions[symbol].erase(hdl);
                if (subscriptions[symbol].empty())
                {
                    subscriptions.erase(symbol);
                }
                std::cout << "Symbol removed from subscriptions: " << symbol << std::endl;
            }

            // End the unsubscribe performance and measure it
            endTradingLoopTiming(metrics);
            metrics.measureTradingLoopLatency();

            json response = {{"status", "success"}, {"message", "Unsubscribed successfully"}};
            startWebSocketTiming(metrics);
            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
            endWebSocketTiming(metrics);
            metrics.measureWebSocketPropagationLatency();
        }
        else if (action == "place_order")
        {
            // Place a new order
            std::string instrument = payload["instrument"];
            std::string price = payload["price"];
            std::string amount = payload["amount"];

            // Start the performance timer for order placement
            startOrderPlacementTiming(metrics);
            placeOrder(price, token, amount, instrument);

            // End the performance timer after order placement
            endOrderPlacementTiming(metrics);
            metrics.measureOrderPlacementLatency();

            json response = {{"status", "success"}, {"message", "Order placed successfully"}};
            startWebSocketTiming(metrics);
            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
            endWebSocketTiming(metrics);
            metrics.measureWebSocketPropagationLatency();
        }
        else if (action == "modify_order")
        {
            std::string orderId = payload["order_id"];
            int amount = payload["amount"];
            double price = payload["price"];

            // Start the performance timer for modifying order
            startOrderPlacementTiming(metrics);
            modifyOrder(token, orderId, amount, price);
            endOrderPlacementTiming(metrics);
            metrics.measureOrderPlacementLatency();

            json response = {{"status", "success"}, {"message", "Order modified successfully"}};
            startWebSocketTiming(metrics);

            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
            endWebSocketTiming(metrics);
            metrics.measureWebSocketPropagationLatency();
        }
        else if (action == "cancel_order")
        {
            std::string orderId = payload["order_id"];

            // Start the performance timer for canceling the order
            startOrderPlacementTiming(metrics);
            cancelOrder(token, orderId);
            endOrderPlacementTiming(metrics);
            metrics.measureOrderPlacementLatency();

            json response = {{"status", "success"}, {"message", "Order cancelled successfully"}};
            startWebSocketTiming(metrics);

            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
            endWebSocketTiming(metrics);
            metrics.measureWebSocketPropagationLatency();
        }
        else if (action == "get_positions")
        {
            std::string instrument = payload["instrument"];
            startMarketDataTiming(metrics);

            getPosition(token, instrument);

            // End the performance timers for market data fetching
            endMarketDataTiming(metrics);
            metrics.measureMarketDataLatency();

            json response = {{"status", "success"}, {"message", "Position details fetched successfully"}};
            startWebSocketTiming(metrics);

            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);

            endWebSocketTiming(metrics);
            metrics.measureWebSocketPropagationLatency();
        }
        else if (action == "get_open_orders")
        {
            // Start measuring the open orders action performance
            startTradingLoopTiming(metrics);

            // Use the existing getOpenOrders function to fetch open orders
            getOpenOrders(token); // You already have this function

            // End the performance measurement for this action
            endTradingLoopTiming(metrics);
            metrics.measureTradingLoopLatency();

            json response = {{"status", "success"}, {"message", "Open orders fetched successfully"}};
            startWebSocketTiming(metrics);

            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);

            endWebSocketTiming(metrics);
            metrics.measureWebSocketPropagationLatency();
        }
        else
        {
            // Invalid action
            json response = {{"status", "error"}, {"message", "Invalid action"}};
            startWebSocketTiming(metrics);

            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
            endWebSocketTiming(metrics);
            metrics.measureWebSocketPropagationLatency();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error processing message: " << e.what() << std::endl;
        json response = {{"status", "error"}, {"message", "Failed to process message"}};
        server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
    }
}

std::string WebSocketOrderBookServer::getValidAccessToken()
{
    if (accessToken.empty())
    {
        std::cout << "Fetching new access token..." << std::endl;
        accessToken = getAccessToken(clientId, clientSecret);
    }
    return accessToken;
}

void WebSocketOrderBookServer::broadcastOrderBook(const std::string &symbol, const std::string &orderBook)
{
    try
    {
        auto parsedOrderBook = json::parse(orderBook);
        if (parsedOrderBook.contains("result"))
        {
            json payload = {
                {"symbol", symbol},
                {"orderBook", parsedOrderBook["result"]}};

            std::lock_guard<std::mutex> lock(subscriptionMutex);
            for (const auto &hdl : subscriptions[symbol])
            {
                server.send(hdl, payload.dump(4), websocketpp::frame::opcode::text);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error broadcasting order book: " << e.what() << std::endl;
    }
}

void WebSocketOrderBookServer::displaySubscribedSymbols()
{
    std::lock_guard<std::mutex> lock(subscriptionMutex);

    std::cout << "Currently Subscribed Symbols:" << std::endl;
    for (const auto &[symbol, clients] : subscriptions)
    {
        std::cout << "- " << symbol << " (" << clients.size() << " clients)" << std::endl;
    }

    if (subscriptions.empty())
    {
        std::cout << "No active subscriptions." << std::endl;
    }
}
