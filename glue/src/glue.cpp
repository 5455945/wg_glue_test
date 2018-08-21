#include <windows.h>
#include "glue-main.h"
#include <atomic>
#include <iostream>
#include <fstream>

std::string g_user_id = "";
std::string g_yilu_device_id = "";
std::string g_yilu_user_id = "";

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32  // is windows
#ifndef _WIN64  // is x86版本；_WIN64 x64版本不需指定
#pragma comment(linker, "/export:RunReload=_RunReload@12")
#pragma comment(linker, "/export:MinerStop=_MinerStop@12")
#pragma comment(linker, "/export:RunWebApi=_RunWebApi@12")
#pragma comment(linker, "/export:GetMinerRate=_GetMinerRate@8")
#pragma comment(linker, "/export:GetUUID=_GetUUID@8")
#pragma comment(linker, "/export:SetUserID=_SetUserID@4")
#pragma comment(linker, "/export:UnSetUserID=_UnSetUserID@4")
#endif
#endif

#ifdef DLL_EXPORT
#define DLL_EXPORTS  extern "C" __declspec(dllexport)
#else
#define DLL_EXPORTS  extern "C" __declspec(dllimport)
#endif

    DLL_EXPORTS int __stdcall RunReload(const char *url, char* body, size_t body_len);
    DLL_EXPORTS int __stdcall MinerStop(const char *url, char* body, size_t body_len);
    DLL_EXPORTS int __stdcall RunWebApi(const char *url, char* body, size_t body_len);
    DLL_EXPORTS bool __stdcall GetMinerRate(double &xmr, double &eth);
    DLL_EXPORTS bool __stdcall GetUUID(char*  uuid, char*  device_name);
    DLL_EXPORTS bool __stdcall SetUserID(const char* user_id);
    DLL_EXPORTS bool __stdcall UnSetUserID(const char* user_id);

    BOOL APIENTRY DllMain(
        HMODULE hModule,
        DWORD  ul_reason_for_call,
        LPVOID lpReserved
    ) {
        switch (ul_reason_for_call)
        {
        case DLL_PROCESS_ATTACH:
            OutputDebugStringA("glue DLL_PROCESS_ATTACH");
            break;
        case DLL_THREAD_ATTACH:
            OutputDebugStringA("glue DLL_THREAD_ATTACH");
            break;
        case DLL_THREAD_DETACH:
            OutputDebugStringA("glue DLL_THREAD_DETACH");
            break;
        case DLL_PROCESS_DETACH:
            OutputDebugStringA("glue DLL_PROCESS_DETACH");
            break;
        }

        return TRUE;
    }

    int __stdcall RunReload(const char *url, char* body, size_t max_size) {
        if (url == nullptr || body == nullptr) {
            return 0;
        }
        std::string sbody;
        bool ret = false;
        ret = reload(url, sbody);
        if ((sbody.length() == 0) || (max_size <= 1)) {
            ret = 0;
        }
        else if (sbody.length() > max_size) {
            memcpy(body, sbody.c_str(), max_size -1);
            ret = max_size - 1;
        }
        else {
            memcpy(body, sbody.c_str(), sbody.length());
            ret = sbody.length();
        }

        OutputDebugStringA("RunReload call");
        return ret;
    }

    int __stdcall MinerStop(const char *url, char* body, size_t max_size) {
        if (url == nullptr || body == nullptr) {
            return 0;
        }
        std::string sbody;
        bool ret = false;
        ret = stop(url, sbody);
        if ((sbody.length() == 0) || (max_size <= 1)) {
            ret = 0;
        }
        else if (sbody.length() > max_size) {
            memcpy(body, sbody.c_str(), max_size - 1);
            ret = max_size - 1;
        }
        else {
            memcpy(body, sbody.c_str(), sbody.length());
            ret = sbody.length();
        }

        OutputDebugStringA("MinerStop call");
        return ret;
    }
 
    int __stdcall RunWebApi(const char *url, char* body, size_t body_len) {
        if (url == nullptr || body == nullptr || body_len <= 1) {
            return 0;
        }
        std::string sbody;
        int ret = 0;
        run_web_api(url, sbody);
        if (sbody.length() == 0) {
            ret = 0;
        }
        else if (sbody.length() > body_len) {
            memcpy(body, sbody.c_str(), body_len - 1);
            ret = body_len - 1;
        }
        else {
            memcpy(body, sbody.c_str(), sbody.length());
            ret = sbody.length();
        }

        OutputDebugStringA("RunWebApi call");
        return ret;
    }

    bool __stdcall GetMinerRate(double &xmr, double &eth) {
        OutputDebugStringA("GetMinerRate call");
        return get_miner_rate(xmr, eth);
    }

    bool __stdcall GetUUID(char* uuid, char* device_name) {
        if (uuid == nullptr || device_name == nullptr) {
            return false;
        }
        std::string suuid;
        std::string sdevice_name;
        bool ret = get_system_uuid(suuid, sdevice_name);
        if (ret) {
            memcpy(uuid, suuid.c_str(), suuid.length());
            memcpy(device_name, sdevice_name.c_str(), sdevice_name.length());
        }

        OutputDebugStringA("GetUUID call");
        return ret;
    }

    bool __stdcall SetUserID(const char* UserID) {
        if (UserID == nullptr) {
            return false;
        }
        bool ret = set_user_id(UserID);

        OutputDebugStringA("SetUserID call");
        return ret;
    }

    bool __stdcall UnSetUserID(const char* UserID) {
        if (UserID == nullptr) {
            return false;
        }
        bool ret = un_set_user_id(UserID);

        OutputDebugStringA("UnSetUserID call");
        return ret;
    }

#ifdef __cplusplus
}
#endif
