#pragma once
#include <string>

#ifndef DATA_BUFFER_
#define DATA_BUFFER_
typedef struct data_buffer_ {
	char *readptr;     // 存放所有待发送的数据
	char *delptr;      // 存放数据的原始内存指针地址，释放内存使用
	size_t data_size;  // 待发送数据的长度
	data_buffer_() {
		memset(this, 0, sizeof(DATA_BUFFER));
	}
}DATA_BUFFER, *PDATA_BUFFER;
#endif

class CurlPost{

private:
	std::string ticks;    // 时间戳
	std::string boundary; // 
	std::string ContentLength;

private:
	std::string url;
	std::string content_type;
	DATA_BUFFER data_buffer;
	
	// 获取url请求的 response 头部信息
	static size_t WriteBodyCallback(char *ptr, size_t size, size_t nmemb, std::string &str);
	// 获取url请求的 response [头部 + 主体] 信息
	static size_t WriteHeaderCallback(char *ptr, size_t size, size_t nmemb, std::string &str);
    // 获取rul请求的response信息，json格式的请求
    static size_t WriteJsonData(void *ptr, size_t size, size_t nmemb, void *stream);
	// 向url请求写如request的 post 数据
	static size_t ReadCallback(void *ptr, size_t size, size_t nmemb, void *userp);

    bool os_file_exists(const std::string &filename);
    std::wstring AsciiToUnicode(const std::string& str);
    std::string UnicodeToUtf8(const std::wstring& wstr);
    //std::string AsciiToUtf8(const string & str);
public:
	inline CurlPost(
		std::string url_,
		std::string content_type_ = std::string()
	)
		: url(url_), content_type(content_type_)
	{};

	// 向发送内存添加数据
	bool PrepareDataHeader();
	bool PrepareData(const std::string& name, const std::string& value);
	bool PrepareDataFromFile(const std::string& name, const std::string& filename);
	bool PrepareDataFoot(bool isFile = false);
    bool post(std::string &sheader, std::string &sbody, std::string &err);
    bool post_json(std::string &sheader, std::string &sbody, std::string &err);
    bool PrepareJsonData(const std::string& json);
    
};
