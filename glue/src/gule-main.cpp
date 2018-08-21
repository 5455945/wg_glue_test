#include "glue-main.h"
#include "curl-post.h"
#include "curl-data.h"
#include "curl-get.h"
#include <comdef.h>
#include <Wbemidl.h>
#include <Shlobj.h>
#include "md5.h"
#include <string>
#include <ctime>
#include <sstream>
#include <iostream>
#include <codecvt>
#include <string>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <cstdio>

using namespace std;

#pragma comment(lib, "wbemuuid.lib")
#pragma comment( lib, "shell32.lib")

extern std::string g_user_id;
extern std::string g_yilu_device_id;
extern std::string g_yilu_user_id;


bool get_system_uuid(std::string &uuid, std::string &device_name) {
    bool bRet = false;
    HRESULT hres;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hres))
    {
        std::cout << "Failed to initialize COM library. Error code = 0x"
            << std::hex << hres << std::endl;
        return bRet;
    }

    // 客户端是不能调用这个函数的，否则再多线程环境下会失败，一个进程只能调用一次这个函数
    // 代理会调用这个函数
    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    //hres = CoInitializeSecurity(
    //    NULL,
    //    -1,                          // COM authentication
    //    NULL,                        // Authentication services
    //    NULL,                        // Reserved
    //    RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
    //    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
    //    NULL,                        // Authentication info
    //    EOAC_NONE,                   // Additional capabilities 
    //    NULL                         // Reserved
    //);


    //if (FAILED(hres))
    //{
    //    std::cout << "Failed to initialize security. Error code = 0x"
    //        << std::hex << hres << std::endl;
    //    CoUninitialize();
    //    return bRet;
    //}

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(hres))
    {
        std::cout << "Failed to create IWbemLocator object."
            << " Err code = 0x"
            << std::hex << hres << std::endl;
        CoUninitialize();
        return bRet;
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object 
        &pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
    {
        std::cout << "Could not connect. Error code = 0x"
            << std::hex << hres << std::endl;
        pLoc->Release();
        CoUninitialize();
        return bRet;
    }

    std::cout << "Connected to ROOT\\CIMV2 WMI namespace" << std::endl;


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        std::cout << "Could not set proxy blanket. Error code = 0x"
            << std::hex << hres << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return bRet;
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, get the name of the operating system
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM win32_computersystemproduct"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        std::cout << "Query for operating system name failed."
            << " Error code = 0x"
            << std::hex << hres << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return bRet;
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        // Get the value of the Name property
        hr = pclsObj->Get(L"uuid", 0, &vtProp, 0, 0);
        std::wcout << " OS uuid : " << vtProp.bstrVal << std::endl;
        std::wstring wuuid(vtProp.bstrVal);
        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt;
        uuid = utf8_cvt.to_bytes(wuuid);
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        std::wcout << " OS Name : " << vtProp.bstrVal << std::endl;
        std::wstring wname(vtProp.bstrVal);
        device_name = utf8_cvt.to_bytes(wname);
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // Cleanup
    // ========
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
    return bRet;
}

void device_register(const std::string &user_id, const std::string &uuid, const std::string &device_name, std::string &body)
{
    //URL: 正式接口：https://zdx.app/api
    //方法： POST JSON
    //公共入参： timestamp, code
    //code = md5(除code之外的的基础类型变量，不包括数组和json类型，所有参数的变量名 + 变量值拼接字符串， 按变量名从小到大排序）
    //返回JSON, 1代表查询成功;
    std::string api_url = "https://zdx.app/api";
    std::string device_reg = "/v1/device/reg";
    std::string device_register_url = api_url + device_reg;
    long long current_time = (long long)time(NULL);
    std::string timestamp = std::to_string(current_time);
    std::string name = device_name;
    std::string precode = "nametimestampuser_iduuid" + name + timestamp + user_id + uuid;
    std::string code = MD5(precode).toString();

    CurlData cpRegister(device_register_url, std::string("application/x-www-form-urlencoded; charset=UTF-8"));
    cpRegister.PrepareData("user_id", user_id);
    cpRegister.PrepareData("uuid", uuid);
    cpRegister.PrepareData("name", name);
    cpRegister.PrepareData("timestamp", timestamp);
    cpRegister.PrepareData("code", code);

    std::string sheader;
    std::string sbody;
    std::string err;
    bool bRet = cpRegister.post(sheader, sbody, err);
    body = sbody;

    //std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt;
    //std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new std::codecvt<wchar_t, char, mbstate_t>("chs"));
    //std::wstring wsbody = utf8_cvt.from_bytes(sbody);
    //std::wstring wsheader = utf8_cvt.from_bytes(sheader);
    //std::wstring werr = utf8_cvt.from_bytes(err);
    //std::cout << "sbody: " << sheader << std::endl;
    //std::wcout << "wsbody: " << wsbody << std::endl;
}

void user_login(std::string &username, std::string &password, int login_mode)
{
    // "https://zdx.app/api/v1/member/login?passwd=4a92746629413c686b80e46f5bcd9cc4&phone_number=13818812913&timestamp=1534228891&type=desktop&code=691c149ba6595063ceb73594358486db"
    std::string api_url = "https://zdx.app/api";
    std::string login_url = "/v1/member/login";
    std::string user_login_url = api_url + login_url;
    std::string prepass = "jraZXF0ShigeXcRb" + password;
    std::string passwd = MD5(prepass).toString();
    long long current_time = (long long)time(NULL);
    std::string timestamp = std::to_string(current_time);
    //std::string user_id = "16";
    std::string phone_number = "13818812913";
    std::string type = "desktop";
    //std::string precode = "passwdphone_numbertimestamptypeuser_id" + passwd + phone_number + timestamp + type + user_id;
    std::string precode = "passwdphone_numbertimestamptype" + passwd + phone_number + timestamp + type;
    std::string code = MD5(precode).toString();
    CurlData cdLogin(user_login_url, "application/x-www-form-urlencoded; charset=UTF-8");
    cdLogin.PrepareData("passwd", passwd);
    cdLogin.PrepareData("phone_number", username);
    cdLogin.PrepareData("timestamp", timestamp);
    cdLogin.PrepareData("type", type);
    //cdLogin.PrepareData("user_id", user_id);
    cdLogin.PrepareData("code", code);
    
    std::string sheader;
    std::string sbody;
    std::string err;
    bool bRet = cdLogin.post(sheader, sbody, err);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt;
    std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new std::codecvt<wchar_t, char, mbstate_t>("chs"));
    std::wstring wsbody = utf8_cvt.from_bytes(sbody);
    std::wstring wsheader = utf8_cvt.from_bytes(sheader);
    std::wstring werr = utf8_cvt.from_bytes(err);

    std::wcout << L"bRet:" << bRet << L", sheader:" << wsheader << L", sbody:" << wsbody << L", err:" << werr << std::endl;
}

bool get_miner_rate(double &xmr_rate, double &eth_rate) {
    std::string api_url = "https://zdx.app/api";
    std::string login_url = "/v1/device/miner-rate";
    std::string miner_rate_url = api_url + login_url;
    long long current_time = (long long)time(NULL);
    std::string timestamp = std::to_string(current_time);
    std::string precode = "timestamp" + timestamp;
    std::string code = MD5(precode).toString();
    CurlData cdLogin(miner_rate_url, "application/x-www-form-urlencoded; charset=UTF-8");
    cdLogin.PrepareData("timestamp", timestamp);
    cdLogin.PrepareData("code", code);

    std::string sheader;
    std::string sbody;
    std::string err;
    cdLogin.post(sheader, sbody, err);
    //std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt;
    //std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new std::codecvt<wchar_t, char, mbstate_t>("chs"));
    //std::wstring wsbody = utf8_cvt.from_bytes(sbody);
    //std::wstring wsheader = utf8_cvt.from_bytes(sheader);
    //std::wstring werr = utf8_cvt.from_bytes(err);
    //std::wcout << L"bRet:" << bRet << L", sheader:" << wsheader << L", sbody:" << wsbody << L", err:" << werr << std::endl;

    // {"rt":1,"error":"","data":{"miner_rate":{"xmr":"0.720514253078087160","eth":"30.392934400934296919"}}}
    xmr_rate = 0;
    eth_rate = 0;
    size_t xmr_idx = sbody.find("\"xmr\":");
    size_t eth_idx = sbody.find("\"eth\":");
    size_t end_idx = sbody.find("\"}}}");
    if (sbody.length() <= 12 || 
        xmr_idx <= 0 ||
        eth_idx <= 0 ||
        end_idx <= 0) {
        return false;
    }
    if (eth_idx - xmr_idx - 9 <= 0 ||
        end_idx - eth_idx - 7 <= 0) {
        return false;
    }
    std::string xmr = sbody.substr(xmr_idx + 7, eth_idx - xmr_idx - 9);
    size_t len = sbody.length();
    std::string eth = sbody.substr(eth_idx + 7, end_idx - eth_idx - 7);

    xmr_rate = std::atof(xmr.c_str());
    eth_rate = std::atof(eth.c_str());

    return true;
}

// web api接口
//http://127.0.0.1:2492/api/stats
bool run_web_api(const std::string &url, std::string &body) {
    std::string sheader;
    std::string sbody;
    bool bRet = curl_get_req2(url, sheader, sbody);
    body = sbody;
    return bRet;
}

// 获取设备id接口
bool yilu_info(const std::string user_id, std::string &yilu_user_id, std::string &yilu_device_id) {
    std::string body;
    std::string uuid;
    std::string device_name;
    std::string appdata;
    std::string filename;

    // 本地读uuid
    bool blocal = false;
    appdata = GetLocalAppDataPathA();
    filename = appdata + "\\ZdxBrowser\\User Data\\Default\\UUID";
    std::ifstream in(filename);
    if (in.is_open())
    {
        std::string str;
        in >> str;
        size_t idx = str.find(";");
        if (idx > 0) {
            uuid = str.substr(0, idx);
            device_name = str.substr(idx+1);
            blocal = true;
        }
        in.close();
    }
    else {
        std::cout << "UUID error!" << std::endl;
        return false;
    }
    //if (!blocal) {
    //    bool bRet = get_system_uuid(uuid, device_name);
    //    if (bRet) {
    //        std::string appdata = GetLocalAppDataPathA();
    //        std::string filename = appdata + "\\ZdxBrowser\\User Data\\Default\\UUID";
    //        std::ofstream out(filename, ios::in | ios::out | ios::trunc);
    //        if (!out.is_open())
    //        {
    //            std::cout << "UUID error!" << std::endl;
    //            return false;
    //        }
    //        std::string str = uuid + ";" + device_name;
    //        out << str;
    //        out.close();
    //    }
    //    else {
    //        std::cout << "UUID error!" << std::endl;
    //        return false;
    //    }
    //}

    device_register(user_id, uuid, device_name, body);

    // {"rt":1,"error":"设备已经注册","data":{"yilu_device_id":260696,"yilu_user_id":1,"user_id":16,"device_id":7}}
    // ${user_id}.${device_id}
    // ${user}
    size_t ydid = body.find("\"yilu_device_id\":");
    if (ydid == 0) {
        std::cout << "No access to yilu_user_id" << std::endl;
        return false;
    }
    size_t yuid = body.find("\"yilu_user_id\":");
    size_t uid = body.find("\"user_id\":");
    yilu_device_id = body.substr(ydid + 17, yuid - ydid - 18);
    size_t len = body.length();
    yilu_user_id = body.substr(yuid + 15, uid - yuid - 16);

    // 如果不等，保存
    if (g_yilu_user_id.compare(yilu_user_id) ||
        g_yilu_device_id.compare(yilu_device_id)) {

        appdata = GetLocalAppDataPathA();
        filename = appdata + "\\ZdxBrowser\\User Data\\Default\\yilu";
        std::ofstream out(filename, ios::in | ios::out | ios::trunc);
        if (!out.is_open())
        {
            std::cout << "Error writting file" << std::endl;
            yilu_user_id = "";
            yilu_device_id = "";
            return false;
        }
        out << yilu_user_id << " " << yilu_device_id;
        out.close();

        g_yilu_user_id = yilu_user_id;
        g_yilu_device_id = yilu_device_id;
    }

    return true;
}

// 从本地读一路信息
bool yilu_info_local(std::string &yilu_user_id, std::string &yilu_device_id) {
    if (g_yilu_user_id != "" && g_yilu_device_id != "") {
        yilu_user_id = g_yilu_user_id;
        yilu_device_id = g_yilu_device_id;
        return true;
    }

    std::string appdata = GetLocalAppDataPathA();
    std::string filename = appdata + "\\ZdxBrowser\\User Data\\Default\\yilu";
    std::ifstream in(filename);
    if (!in.is_open())
    {
        // 如果存在g_user_id,到服务器取
        if (g_user_id.length() > 0) {
            bool bRet = yilu_info(g_user_id, yilu_user_id, yilu_device_id);
            return bRet;
        }
        std::cout << "Error opening file" << std::endl;
        yilu_user_id = "";
        yilu_device_id = "";
        return false;
    }
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    in.getline(buffer, 256);
    in.close();

    std::string data = buffer;
    size_t idx = data.find(' ');
    if (idx == 0) {
        std::cout << "Error opening file" << std::endl;
        yilu_user_id = "";
        yilu_device_id = "";
        return false;
    }
    yilu_user_id = data.substr(0, idx);
    yilu_device_id = data.substr(idx + 1);
    std::cout << "local_info: yilu_user_id:" << yilu_user_id << " yilu_device_id:" << yilu_device_id << std::endl;

    return true;
}

std::wstring GetLocalAppDataPath()
{
    wchar_t m_lpszDefaultDir[MAX_PATH];
    wchar_t szDocument[MAX_PATH] = { 0 };
    memset(m_lpszDefaultDir, 0, _MAX_PATH);

    LPITEMIDLIST pidl = NULL;
    SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
    if (pidl && SHGetPathFromIDListW(pidl, szDocument))
    {
        GetShortPathNameW(szDocument, m_lpszDefaultDir, _MAX_PATH);
    }

    std::wstring wsR = m_lpszDefaultDir;

    return wsR;
}

std::string GetLocalAppDataPathA()
{
    char m_lpszDefaultDir[MAX_PATH];
    char szDocument[MAX_PATH] = { 0 };
    memset(m_lpszDefaultDir, 0, _MAX_PATH);

    LPITEMIDLIST pidl = NULL;
    SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
    if (pidl && SHGetPathFromIDListA(pidl, szDocument))
    {
        GetShortPathNameA(szDocument, m_lpszDefaultDir, _MAX_PATH);
    }

    std::string sR = m_lpszDefaultDir;

    return sR;
}

std::wstring GetLocalAppDataPath(std::wstring appName)
{
    std::wstring path = GetLocalAppDataPath();
    path.append(L"\\");
    path.append(appName);

    if (_waccess(path.c_str(), 0) == -1)
    {
        _wmkdir(path.c_str());
    }

    return path;

}

// reload接口
// http://127.0.0.1:2492/api/reload
bool reload(const std::string &url, std::string &body) {
    std::string user_id;
    {
        // 从文件读取user_id
        
        std::string appdata = GetLocalAppDataPathA();
        std::string filename = appdata + "\\ZdxBrowser\\User Data\\Default\\user_id";
        std::ifstream in(filename);
        if (!in.is_open())
        {
            std::cout << "user_id no exists!" << std::endl;
            body = "{\"rt\":-2,\"error\":\"user_id no exists\"}";
            return false;
        }
        in >> user_id;
        in.close();
        g_user_id = user_id;
    }

    std::string yilu_user_id;
    std::string yilu_device_id;
    bool bRet = yilu_info(user_id.c_str(), yilu_user_id, yilu_device_id);
    if (!bRet) {
        std::cout << "yilu info error!" << std::endl;
        body = "{\"rt\":-3,\"error\":\"yilu info error!\"}";
        return false;
    }

    int file_length = 0;
    std::string appdata = GetLocalAppDataPathA();
    std::string filename = appdata + "\\ZdxBrowser\\miner\\work\\w1.json";
    std::fstream frw(filename, ios::in | ios::out | ios::app);
    if (!frw.is_open())
    {
       std::cout << "reload fail, read w1.json error" << std::endl;
       body = "{\"rt\":-4,\"error\":\"read w1.json, error 1\"}";
       return false;
    }

    frw.seekg(0, std::ios::end);
    file_length = (int)frw.tellg();
    frw.seekg(0, std::ios::beg);
    char buffer[4096];
    memset(buffer, 0, 4096);
    if (file_length > 4095) {
        std::cout << "reload fail, read w1.json error2" << std::endl;
        body = "{\"rt\":-5,\"error\":\"read w1.json, error 2\"}";
        return false;
    }
    frw.read(buffer, file_length);
    frw.close();

    std::string buf = buffer;
    std::string info = yilu_user_id + "." + yilu_device_id;
    string_replase(buf, "${user_id}.${device_id}", info);
    std::string info1 = yilu_user_id + "_" + yilu_device_id;
    string_replase(buf, "${user}", info1);
    filename = appdata + "\\ZdxBrowser\\miner\\work\\workers.json";
    std::ofstream fw(filename, ios::out | ios::trunc);
    if (!fw.is_open())
    {
        std::cout << "reload fail, write workers.json error" << std::endl;
        body = "{\"rt\":-5,\"error\":\"write workers.json error\"}";
        return false;
    }
    fw << buf;
    fw.flush();
    frw.close();

    std::string sheader;
    std::string sbody;
    bRet = curl_get_req2(url, sheader, sbody);
    if (sbody.length() == 0) {
        body = "{\"rt\":0,\"error\":\"\"}";
        bRet = false;
    }
    else {
        body = sbody;
    }
    
    //wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt;
    //wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs"));
    //wstring t = gbk_cvt.from_bytes(sbody);
    //body = utf8_cvt.to_bytes(t);

    return bRet;
}

bool stop(const std::string &url, std::string &body)
{
    {
        // 从文件读取user_id
        std::string user_id;
        std::string appdata = GetLocalAppDataPathA();
        std::string filename = appdata + "\\ZdxBrowser\\User Data\\Default\\user_id";
        std::ifstream in(filename);
        if (!in.is_open())
        {
            std::cout << "user_id no exists!" << std::endl;
            body = "{\"rt\":-2,\"error\":\"user_id no exists\"}";
            return false;
        }
        in >> user_id;
        in.close();
        g_user_id = user_id;
    }

    std::string yilu_user_id;
    std::string yilu_device_id;
    bool bRet = yilu_info(g_user_id, yilu_user_id, yilu_device_id);
    if (!bRet) {
        std::cout << "yilu info error!" << std::endl;
        body = "{\"rt\":-3,\"error\":\"yilu info error!\"}";
        return false;
    }

    int file_length = 0;
    std::string appdata = GetLocalAppDataPathA();
    std::string filename = appdata + "\\ZdxBrowser\\miner\\work\\w2.json";
    std::fstream frw(filename, ios::in | ios::out | ios::app);
    if (!frw.is_open())
    {
        std::cout << "reload fail, read w1.json error" << std::endl;
        body = "{\"rt\":-4,\"error\":\"read w2.json, error 1\"}";
        return false;
    }

    frw.seekg(0, std::ios::end);
    file_length = (int)frw.tellg();
    frw.seekg(0, std::ios::beg);
    char buffer[4096];
    memset(buffer, 0, 4096);
    if (file_length > 4095) {
        std::cout << "reload fail, read w2.json error2" << std::endl;
        body = "{\"rt\":-5,\"error\":\"read w2.json, error 2\"}";
        return false;
    }
    frw.read(buffer, file_length);
    frw.close();

    std::string buf = buffer;
    std::string info = yilu_user_id + "." + yilu_device_id;
    string_replase(buf, "${user_id}.${device_id}", info);
    std::string info1 = yilu_user_id + "_" + yilu_device_id;
    string_replase(buf, "${user}", info1);
    filename = appdata + "\\ZdxBrowser\\miner\\work\\workers.json";
    std::ofstream fw(filename, ios::out | ios::trunc);
    if (!fw.is_open())
    {
        std::cout << "reload fail, write workers.json error" << std::endl;
        body = "{\"rt\":-5,\"error\":\"write workers.json error\"}";
        return false;
    }
    fw << buf;
    fw.flush();
    frw.close();

    std::string sheader;
    std::string sbody;
    bRet = curl_get_req2(url, sheader, sbody);
    if (sbody.length() == 0) {
        body = "{\"rt\":0,\"error\":\"\"}";
        bRet = false;
    }
    else {
        body = sbody;
    }

    //wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt;
    //wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs"));
    //wstring t = gbk_cvt.from_bytes(sbody);
    //body = utf8_cvt.to_bytes(t);

    return bRet;
}


int string_replase(string &s1, const string &s2, const string &s3)
{
    string::size_type pos = 0;
    string::size_type a = s2.size();
    string::size_type b = s3.size();
    while ((pos = s1.find(s2, pos)) != string::npos)
    {
        s1.replace(pos, a, s3);
        pos += b;
    }
    return 0;
}

bool set_user_id(const std::string &user_id)
{
    std::string appdata = GetLocalAppDataPathA();
    std::string filename = appdata + "\\ZdxBrowser\\User Data\\Default\\user_id";
    std::ofstream out(filename, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
       return false;
    }
    out << user_id;
    out.close();
    return true;
}

bool un_set_user_id(const std::string &user_id)
{
    
    std::string appdata = GetLocalAppDataPathA();
    std::string filename = appdata + "\\ZdxBrowser\\User Data\\Default\\user_id";
    int nRet = std::remove(filename.c_str());
    if (nRet == 0) {
        std::cout << "remove file success: " << filename << std::endl;
    }
    else {
        std::cout << "remove file error: " << filename << std::endl;
    }
    return (nRet == 0);
}


//int main(int argc, char* argv[])
//{
//    //std::string username = "13818812913";
//    //std::string password = "Zhangfj@123";
//    //int login_mode = 0;
//    //user_login(username, password, login_mode);
//
//    //std::string uuid;
//    //std::string device_name;
//    //std::string user_id = "16";
//    //std::string body;
//
//    //get_system_uuid(uuid, device_name);
//    //device_register(user_id, uuid, device_name, body);
//
//    //// {"rt":1,"error":"设备已经注册","data":{"yilu_device_id":260696,"yilu_user_id":1,"user_id":16,"device_id":7}}
//    //// ${user_id}.${device_id}
//    //// ${user}
//    //size_t ydid = body.find("\"yilu_device_id\":");
//    //size_t yuid = body.find("\"yilu_user_id\":");
//    //size_t uid = body.find("\"user_id\":");
//    //std::string yilu_device_id = body.substr(ydid + 17, yuid - ydid - 18 );
//    //size_t len = body.length();
//    //std::string yilu_user_id = body.substr(yuid + 15, uid - yuid - 16);
//
//    //std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt;
//    //std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new std::codecvt<wchar_t, char, mbstate_t>("chs"));
//    //std::wstring wsbody = utf8_cvt.from_bytes(body);
//
//    //g_user_id = "16";
//    //std::string yilu_user_id;
//    //std::string yilu_device_id;
//
//    //bool ret = yilu_info_local(yilu_user_id, yilu_device_id);
//
    //{
    //    std::string body;
    //    bool ret = reload(body);
    //}
//
//    //{
//    //    std::string body;
//    //    bool ret = get_stats(body);
//    //}
//
//    return 0;
//}
