#include "websocket_server.h"
#include "/Users/omvibhandik/Desktop/deribit_oems/src/api/deribit_api.h" 
#include <iostream>
#include <thread>
#include <chrono>
#include "/Users/omvibhandik/Desktop/deribit_oems/src/include/json.hpp"

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
    try
    {
        auto payload = json::parse(msg->get_payload());
        std::string action = payload["action"];
        std::string token = getValidAccessToken(); 

        if (action == "subscribe")
        {
            std::string symbol = payload["symbol"];
            std::cout << "Client subscribed to " << symbol << std::endl;

            // Add the symbol to the subscription list for this client
            {
                std::lock_guard<std::mutex> lock(subscriptionMutex);
                subscriptions[symbol].insert(hdl);
                std::cout << "Symbol added to subscriptions: " << symbol << std::endl;
            }

            std::cout << "Client subscribed to " << symbol << std::endl;

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

            json responseJson = json::parse(response);
            server.send(hdl, response, websocketpp::frame::opcode::text);
        }
        else if (action == "unsubscribe")
        {
            std::string symbol = payload["symbol"];
            std::cout << "Client unsubscribed from " << symbol << std::endl;

            {
                std::lock_guard<std::mutex> lock(subscriptionMutex);
                subscriptions[symbol].erase(hdl);
                if (subscriptions[symbol].empty())
                {
                    subscriptions.erase(symbol); 
                }
                std::cout << "Symbol removed from subscriptions: " << symbol << std::endl;
            }
        }
        else if (action == "place_order")
        {
            // Place a new order
            std::string instrument = payload["instrument"];
            std::string price = payload["price"];
            std::string amount = payload["amount"];
            placeOrder(price, token, amount, instrument);

            json response = {{"status", "success"}, {"message", "Order placed successfully"}};
            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
        }
        else if (action == "modify_order")
        {
            std::string orderId = payload["order_id"];
            int amount = payload["amount"];
            double price = payload["price"];
            modifyOrder(token, orderId, amount, price);

            json response = {{"status", "success"}, {"message", "Order modified successfully"}};
            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
        }
        else if (action == "cancel_order")
        {
            std::string orderId = payload["order_id"];
            cancelOrder(token, orderId);

            json response = {{"status", "success"}, {"message", "Order cancelled successfully"}};
            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
        }
        else if (action == "get_open_orders")
        {
            getOpenOrders(token);

            json response = {{"status", "success"}, {"message", "Open orders fetched successfully"}};
            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
        }
        else if (action == "get_positions")
        {
            std::string instrument = payload["instrument"];
            getPosition(token, instrument);

            json response = {{"status", "success"}, {"message", "Position details fetched successfully"}};
            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
        }
        else
        {
            json response = {{"status", "error"}, {"message", "Invalid action"}};
            server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
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
