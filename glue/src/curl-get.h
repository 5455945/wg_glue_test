#pragma once
#include <string>

bool curl_get_req(const std::string &url, std::string &sBody, std::string &sHeader);

bool curl_get_req2(const std::string &url, std::string &sHeader, std::string &sBody);
