#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <string>
#include <unordered_map>
#include <mutex>

typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;

class WebSocketOrderBookServer
{
public:
    WebSocketOrderBookServer(const std::string &clientId, const std::string &clientSecret);
    void run(uint16_t port);
    void startOrderBookUpdates(int intervalSeconds);
    void displaySubscribedSymbols();
    void broadcastOrderBook(const std::string &symbol, const std::string &orderBook);

private:
    WebSocketServer server;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> clients;
    std::unordered_map<std::string, std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>>> subscriptions;
    std::mutex subscriptionMutex;

    std::string clientId;
    std::string clientSecret;
    std::string accessToken;

    void onOpen(websocketpp::connection_hdl hdl);
    void onClose(websocketpp::connection_hdl hdl);
    void onMessage(websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg);
    std::string getValidAccessToken();
    
};

#endif 
