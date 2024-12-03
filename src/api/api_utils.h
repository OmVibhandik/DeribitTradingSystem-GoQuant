#ifndef API_UTILS_H
#define API_UTILS_H

#include <string>
#include "/Users/omvibhandik/Desktop/deribit_oems/src/include/json.hpp"

using json = nlohmann::json;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);


std::string sendRequest(const std::string &url, const json &payload, const std::string &accessToken = "");

#endif 
