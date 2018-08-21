#pragma once
#include <string>

bool get_system_uuid(std::string &uuid, std::string &device_name);

void device_register(const std::string &user_id, const std::string &uuid, const std::string &device_name, std::string &body);

void user_login(std::string &username, std::string &password, int login_mode = 0);

bool reload(const std::string &url, std::string &body);

bool stop(const std::string &url, std::string &body);

bool set_user_id(const std::string &user_id);

bool un_set_user_id(const std::string &user_id);

bool run_web_api(const std::string &url, std::string &body);

bool get_miner_rate(double &xmr, double &eth);

bool yilu_info(const std::string user_id, std::string &yilu_user_id, std::string &yilu_device_id);

bool yilu_info_local(std::string &yilu_user_id, std::string &yilu_device_id);

std::wstring GetLocalAppDataPath();

std::string GetLocalAppDataPathA();

std::wstring GetLocalAppDataPath(std::wstring appName);

int string_replase(std::string &s1, const std::string &s2, const std::string &s3);

