#include <windows.h>
#include <Shlobj.h>
#include <tchar.h>
#include <thread>
#include <string>
#include <iostream>
#include <fstream>
#pragma comment(lib, "shell32.lib")

typedef int  (__stdcall *pFunRunReload)(const char *url, char* body, size_t body_len);
typedef int  (__stdcall *pFunRunWebApi)(const char *url, char* body, size_t body_len);
typedef int  (__stdcall *pFunMinerStop)(const char *url, char* body, size_t body_len);
typedef bool (__stdcall *pFunGetMinerRate)(double &xmr_rate, double &eth_rate);
typedef bool (__stdcall *pFunGetUUID)(char*  uuid, char*  device_name);
typedef bool (__stdcall *pFunSetUserID)(const char* UserID);
typedef bool (__stdcall *pFunUnSetUserID)(const char* UserID);

int test_RunReload(const std::string &url, std::string &body) {
    int ret = 0;
    HINSTANCE hApp = ::LoadLibraryA("glue.dll");
    if (!hApp) {
        std::cout << "LoadLibraryA glue.dll error!!" << std::endl;
        return -1;
    }

    pFunRunReload pRunReload = (pFunRunReload)::GetProcAddress(hApp, "RunReload");
    if (!pRunReload) {
        std::cout << "GetProcAddress RunReload function error!" << std::endl;
        if (hApp) {
            FreeLibrary(hApp);
            hApp = nullptr;
        }
        return -2;
    }

    if (pRunReload) {
        char sbody[4096];
        memset(sbody, 0, 4096);
        size_t sbody_len = 4096;
        ret = pRunReload(url.c_str(), sbody, sbody_len);
        if (ret > 0) {
            body = sbody;
            std::cout << "RunReload success: " << sbody << std::endl;
        }
        else {
            std::cout << "RunReload error: " << std::endl;
        }
    }

    if (hApp) {
        FreeLibrary(hApp);
        hApp = nullptr;
    }
    return ret;
}

int test_MinerStop(const std::string &url, std::string &body) {
    int ret = 0;
    HINSTANCE hApp = ::LoadLibraryA("glue.dll");
    if (!hApp) {
        std::cout << "LoadLibraryA glue.dll error!!" << std::endl;
        return -1;
    }

    pFunMinerStop pMinerStop = (pFunMinerStop)::GetProcAddress(hApp, "MinerStop");
    if (!pMinerStop) {
        std::cout << "GetProcAddress RunStop function error!" << std::endl;
        if (hApp) {
            FreeLibrary(hApp);
            hApp = nullptr;
        }
        return -2;
    }

    if (pMinerStop) {
        char sbody[4096];
        memset(sbody, 0, 4096);
        size_t sbody_len = 4096;
        ret = pMinerStop(url.c_str(), sbody, sbody_len);
        if (ret > 0) {
            body = sbody;
            std::cout << "RunStop success: " << sbody << std::endl;
        }
        else {
            std::cout << "RunStop error: " << std::endl;
        }
    }

    if (hApp) {
        FreeLibrary(hApp);
        hApp = nullptr;
    }
    return ret;
}

int test_RunWebApi(const std::string &url, std::string &body) {
    int ret = 0;
    HINSTANCE hApp = ::LoadLibraryA("glue.dll");
    if (!hApp) {
        std::cout << "LoadLibraryA glue.dll error!!" << std::endl;
        return -1;
    }

    pFunRunWebApi pRunWebApi = (pFunRunWebApi)::GetProcAddress(hApp, "RunWebApi");
    if (!pRunWebApi) {
        std::cout << "GetProcAddress run_reload function error!" << std::endl;
        if (hApp) {
            FreeLibrary(hApp);
            hApp = nullptr;
        }
        return -2;
    }

    if (pRunWebApi) {
        char sbody[4096];
        memset(sbody, 0, 4096);
        size_t sbody_len = 4096;
        ret = pRunWebApi(url.c_str(), sbody, sbody_len);
        if (ret > 0) {
            body = sbody;
            std::cout << "RunWebApi success: " << body << std::endl;
        }
        else {
            std::cout << "RunWebApi error: " << std::endl;
        }
    }

    if (hApp) {
        FreeLibrary(hApp);
        hApp = nullptr;
    }

    return ret;
}

bool test_GetMinerRate(double& xmr_rate, double &eth_rate) {
    HINSTANCE hApp = ::LoadLibraryA("glue.dll");
    if (!hApp) {
        std::cout << "LoadLibraryA glue.dll error!!" << std::endl;
        return false;
    }

    pFunGetMinerRate pGetMinerRate = (pFunGetMinerRate)::GetProcAddress(hApp, "GetMinerRate");
    if (!pGetMinerRate) {
        std::cout << "GetProcAddress get_miner_rate function error!" << std::endl;
        if (hApp) {
            FreeLibrary(hApp);
            hApp = nullptr;
        }
        return false;
    }

    bool bRet = false;
    if (pGetMinerRate) {
        bRet = pGetMinerRate(xmr_rate, eth_rate);
        if (bRet) {
            std::cout << "GetMinerRate success: xmr_rate:" << xmr_rate << ", eth_rate: " << eth_rate << std::endl;
        }
        else {
            std::cout << "GetMinerRate error: " << std::endl;
        }
    }

    if (hApp) {
        FreeLibrary(hApp);
        hApp = nullptr;
    }

    return bRet;
}

bool test_GetUUID(char*  uuid, char*  device_name) {
    HINSTANCE hApp = ::LoadLibraryA("glue.dll");
    if (!hApp) {
        std::cout << "LoadLibraryA glue.dll error!!" << std::endl;
        return false;
    }

    pFunGetUUID pGetUUID = (pFunGetUUID)::GetProcAddress(hApp, "GetUUID");
    if (!pGetUUID) {
        std::cout << "GetProcAddress get_miner_rate function error!" << std::endl;
        if (hApp) {
            FreeLibrary(hApp);
            hApp = nullptr;
        }
        return false;
    }

    bool bRet = false;
    if (pGetUUID) {
        bRet = pGetUUID(uuid, device_name);
        if (bRet) {
            std::cout << "GetUUID success: UUID:" << uuid << ", device_name: " << device_name << std::endl;
        }
        else {
            std::cout << "GetUUID error: " << std::endl;
        }
    }

    if (hApp) {
        FreeLibrary(hApp);
        hApp = nullptr;
    }

    return bRet;
}


bool test_SetUserID(const char* UserID) {
    HINSTANCE hApp = ::LoadLibraryA("glue.dll");
    if (!hApp) {
        std::cout << "LoadLibraryA glue.dll error!!" << std::endl;
        return false;
    }

    pFunSetUserID pSetUserID = (pFunSetUserID)::GetProcAddress(hApp, "SetUserID");
    if (!pSetUserID) {
        std::cout << "GetProcAddress SetUserID function error!" << std::endl;
        if (hApp) {
            FreeLibrary(hApp);
            hApp = nullptr;
        }
        return false;
    }

    bool bRet = false;
    if (pSetUserID) {
        bRet = pSetUserID(UserID);
        std::cout << "SetUserID: " << (bRet ? "成功" : "失败") << std::endl;
    }

    if (hApp) {
        FreeLibrary(hApp);
        hApp = nullptr;
    }

    return bRet;
}

bool test_UnSetUserID(const char* UserID) {
    HINSTANCE hApp = ::LoadLibraryA("glue.dll");
    if (!hApp) {
        std::cout << "LoadLibraryA glue.dll error!!" << std::endl;
        return false;
    }

    pFunUnSetUserID pUnSetUserID = (pFunUnSetUserID)::GetProcAddress(hApp, "UnSetUserID");
    if (!pUnSetUserID) {
        std::cout << "GetProcAddress UnSetUserID function error!" << std::endl;
        if (hApp) {
            FreeLibrary(hApp);
            hApp = nullptr;
        }
        return false;
    }

    bool bRet = false;
    if (pUnSetUserID) {
        bRet = pUnSetUserID(UserID);
        std::cout << "UnSetUserID: " << (bRet ? "成功" : "失败") << std::endl;
    }

    if (hApp) {
        FreeLibrary(hApp);
        hApp = nullptr;
    }

    return bRet;
}

typedef bool(__stdcall *pFunGetUUID)(char*  uuid, char*  device_name);

int _tmain(int argc, _TCHAR* argv[])
{
    int nRet = 0;
    bool bRet = false;
    std::string body;

    //// 这个测试需要在 %appdata%\ZdxBrowser\User Data\Default
    //// 即 C:\Users\soft\AppData\Roaming\ZdxBrowser\User Data\Default
    //// 下面有 user_id 里面有用户id值
    //// 然后，test_RunReload会获取一路赚钱的设备id也用户id
    bRet = test_SetUserID("16");
    bRet = test_UnSetUserID("16");
    bRet = test_SetUserID("16");

    // reload cpu
    nRet = test_RunReload("http://localhost:2492/api/reload?dev=cpu", body);

    //// reload amd
    //nRet = test_RunReload("http://localhost:2492/api/reload?dev=agpu", body);

    //// reload nvidia
    //nRet = test_RunReload("http://localhost:2492/api/reload?dev=ngpu", body);

    //nRet = test_RunWebApi("http://localhost:2492/api/reload?dev=ngpu", body);

    //double xmr_rate = 0;
    //double eth_rate = 0;
    //bRet = test_GetMinerRate(xmr_rate, eth_rate);

    // stop cpu
    nRet = test_MinerStop("http://localhost:2492/api/reload?dev=cpu", body);

    char uuid[256];
    char device_name[256];
    memset(uuid, 0, 256);
    memset(device_name, 0, 256);
    nRet = test_GetUUID(uuid, device_name);
    if (nRet) {
        std::cout << "GetUUID success: UUID:" << uuid << ", device_name: " << device_name << std::endl;
    }
    else {
        std::cout << "GetUUID error: " << std::endl;
    }


    return 0;
}
