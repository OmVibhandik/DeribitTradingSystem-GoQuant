#ifndef DERIBIT_API_H
#define DERIBIT_API_H

#include <string>
#include "api_utils.h"

std::string getAccessToken(const std::string &clientId, const std::string &clientSecret);
void getOrderBook(const std::string &accessToken, const std::string &instrument);
void modifyOrder(const std::string &accessToken, const std::string &orderID, int amount, double price);
void getPosition(const std::string &accessToken, const std::string &instrument);
void getOpenOrders(const std::string &accessToken);
void placeOrder(const std::string &price, const std::string &accessToken, const std::string &amount, const std::string &instrument);
void cancelOrder(const std::string &accessToken, const std::string &orderID);

#endif 
