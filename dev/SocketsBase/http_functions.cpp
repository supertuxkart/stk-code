#include "http_functions.hpp"


#include <iostream>
#include <string>
#include <curl/curl.h>

namespace HTTP
{
CURL *curl;
CURLcode res;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl)
        printf("Error while loading cURL library.\n");
}

std::string getPage(std::string url)
{
    std::string readBuffer = "";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    readBuffer.erase(readBuffer.begin()+64, readBuffer.end());
    return readBuffer;
}

void shutdown()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

}
