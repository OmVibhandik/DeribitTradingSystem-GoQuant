#include "api_utils.h"
#include <iostream>
#include <curl/curl.h>
#include <chrono>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string sendRequest(const std::string &url, const json &payload, const std::string &accessToken)
{
    auto startApiTime = std::chrono::high_resolution_clock::now(); // Start time for API response measurement

    std::string readBuffer;
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        std::string jsonStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        struct curl_slist *headers = NULL;


        headers = curl_slist_append(headers, "Content-Type: application/json");


        if (!accessToken.empty())
        {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        }


        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);


        res = curl_easy_perform(curl);
        // std::cout << "Heelllo\n";

        if (res != CURLE_OK)
        {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    
    auto endApiTime = std::chrono::high_resolution_clock::now(); 
    auto apiDuration = std::chrono::duration_cast<std::chrono::microseconds>(endApiTime - startApiTime).count();

    std::cout << "API Response Time: " << apiDuration << " microseconds" << std::endl;

    return readBuffer;
}