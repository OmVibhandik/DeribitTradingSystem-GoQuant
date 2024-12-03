#include "deribit_api.h"
#include <iostream>

std::string getAccessToken(const std::string &clientId, const std::string &clientSecret)
{
    json payload = {
        {"id", 0},
        {"method", "public/auth"},
        {"params", {{"grant_type", "client_credentials"}, {"scope", "session:apiconsole"}, {"client_id", clientId}, {"client_secret", clientSecret}}},
        {"jsonrpc", "2.0"}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/auth", payload);
    std::cout << "Raw API Response: " << response << std::endl; 

    try
    {
        auto responseJson = json::parse(response);

        if (responseJson.contains("result") && responseJson["result"].contains("access_token"))
        {
            return responseJson["result"]["access_token"];
        }
        else
        {
            std::cerr << "Error in API response: " << responseJson.dump(4) << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
    }

    return ""; 
}

void getOrderBook(const std::string &accessToken, const std::string &instrument)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "public/get_order_book"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 15}};

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book", payload, accessToken);
    auto responseJson = json::parse(response);

    if (responseJson.contains("result"))
    {
        std::cout << "Order Book for " << instrument << ":\n";
        std::cout << "Best Bid: " << responseJson["result"]["best_bid_price"]
                  << ", Amount: " << responseJson["result"]["best_bid_amount"] << "\n";
        std::cout << "Best Ask: " << responseJson["result"]["best_ask_price"]
                  << ", Amount: " << responseJson["result"]["best_ask_amount"] << "\n";
    }
    else
    {
        std::cerr << "Failed to retrieve order book." << std::endl;
    }
}

void modifyOrder(const std::string &accessToken, const std::string &orderID, int amount, double price)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/edit"},
        {"params", {{"order_id", orderID}, {"amount", amount}, {"price", price}}},
        {"id", 11}};

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/edit", payload, accessToken);
    std::cout << "Modify Order Response: " << response << std::endl;
}

void getPosition(const std::string &accessToken, const std::string &instrument)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_position"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 20}};

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_position", payload, accessToken);

    std::cout << "Raw API Response (Get Position): " << response << std::endl;

    try
    {
        auto responseJson = json::parse(response);
        if (responseJson.contains("result"))
        {
            const auto &position = responseJson["result"];
            std::cout << "Position Details:\n"
                      << position.dump(4) << std::endl;
        }
        else if (responseJson.contains("error"))
        {
            std::cerr << "Error fetching position: " << responseJson["error"]["message"] << std::endl;
        }
        else
        {
            std::cerr << "Unexpected response format: " << responseJson.dump(4) << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing response: " << e.what() << std::endl;
    }
}

void getOpenOrders(const std::string &accessToken)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_open_orders"},
        {"params", {{"kind", "future"}}}, 
        {"id", 25}};

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_open_orders", payload, accessToken);

    std::cout << "Raw API Response (Get Open Orders): " << response << std::endl;

    try
    {

        // std::cout<<"Hello\n";
        auto responseJson = json::parse(response);
        if (responseJson.contains("result"))
        {
            const auto &orders = responseJson["result"];
            std::cout << "Open Orders:\n"
                      << orders.dump(4) << std::endl; 
        }
        else if (responseJson.contains("error"))
        {
            std::cerr << "Error fetching open orders: " << responseJson["error"]["message"] << std::endl;
        }
        else
        {
            std::cerr << "Unexpected response format: " << responseJson.dump(4) << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing response: " << e.what() << std::endl;
    }
}

void placeOrder(const std::string &price, const std::string &accessToken, const std::string &amount, const std::string &instrument)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/buy"},
        {"params", {{"instrument_name", instrument}, {"type", "limit"}, {"price", price}, {"amount", amount}}},
        {"id", 1}};

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/buy", payload, accessToken);
    std::cout << "Raw Place Order Response: " << response << std::endl;

    try
    {
        auto responseJson = json::parse(response);

        if (responseJson.contains("result"))
        {
            std::cout << "Order placed successfully: " << responseJson["result"].dump(4) << std::endl;
        }
        else if (responseJson.contains("error"))
        {
            std::cerr << "Error placing order: " << responseJson["error"]["message"] << std::endl;
        }
        else
        {
            std::cerr << "Unexpected response format: " << responseJson.dump(4) << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing response: " << e.what() << std::endl;
    }
}

void cancelOrder(const std::string &accessToken, const std::string &orderID)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/cancel"},
        {"params", {{"order_id", orderID}}},
        {"id", 6}};

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/cancel", payload, accessToken);
    std::cout << "Cancel Order Response: " << response << std::endl;
}
