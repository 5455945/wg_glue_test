#include <iostream>  
#include <string>  
#include "curl/curl.h"  
using namespace std;

#pragma comment(lib, "ws2_32.lib")  
#pragma comment(lib, "wldap32.lib")  
#pragma comment(lib, "libcurl.lib")  

// WIN32 NDEBUG
// https://blog.csdn.net/u013317006/article/details/20696261

// https://blog.csdn.net/xuheazx/article/details/52689327
//libcurl ��֧���첽 dns ����ʱ����ͨ�� signal �ķ�ʽʵ�� dns �������ó�ʱ�� 
//signal �ᵼ�¶��̳߳����������̨����ͨ�����Ƕ��̵߳ģ�
//����Ӧ�������������ѡ����� libcurl ��֧���첽 dns ����ʱ����ʱѡ������ԣ���
//����ͨ������ curl --version �������� curl_version �����鿴 libcurl �Ƿ�֧���첽 dns ������
//���� curl_version_info ���������Ի�þ���� c - ares ��汾�š�
//���� libcurl ʱ��ͨ��Ϊ configure ָ�� --enable - threaded - resolver �� --enable - ares ѡ�������첽 dns ������

// https://blog.csdn.net/delphiwcdj/article/details/18284429

size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream)
{
    string *str = (string*)stream;
    cout << *str << endl;
    (*str).append((char*)ptr, size*nmemb);
    return size * nmemb;
}

// ���������release �汾�����˳�
bool curl_get_req(const std::string &url, std::string &sHeader, std::string &sBody)
{
    CURL *curl = curl_easy_init();
    CURLcode res;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&sBody);
        sHeader = "";
        
        res = curl_easy_perform(curl);
    }
    
    curl_easy_cleanup(curl);
    return true;
}

// ����ģʽdebug/release������
bool curl_get_req2(const std::string &url, std::string &sHeader, std::string &sBody)
{
    auto curl_deleter = [](CURL *curl) {curl_easy_cleanup(curl); };
    using Curl = unique_ptr<CURL, decltype(curl_deleter)>;
    Curl curl{ curl_easy_init(), curl_deleter };
    CURLcode res;
    if (curl)
    {
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, false);
        curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_HEADER, 0);
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 30);
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_TIME, 5);
        #ifdef CURL_DOES_CONVERSIONS
                curl_easy_setopt(curl.get(), CURLOPT_TRANSFERTEXT, 1L);
        #endif
        #if LIBCURL_VERSION_NUM >= 0x072400
                curl_easy_setopt(curl.get(), CURLOPT_SSL_ENABLE_ALPN, 0L);
        #endif
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, req_reply);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, (void *)&sBody);
        sHeader = "";
        res = curl_easy_perform(curl.get());
    }

    return true;
}
