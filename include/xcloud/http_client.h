#pragma once
#include <string>
#include <map>

namespace xcloud {

struct HttpResponse {
    long status = 0;       // HTTP 状态码，0 表示请求层失败
    std::string body;
    std::string error;     // 非空表示 libcurl / 传输层错误
};

// 极简同步 HTTP 客户端（libcurl 封装）。
// Switch 端可替换为 libnx 的 HTTP 后端，接口不变。
class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    void setUserAgent(const std::string& ua);
    void setTimeoutSec(int sec);

    HttpResponse get(const std::string& url,
                     const std::map<std::string, std::string>& headers = {});
    HttpResponse post(const std::string& url,
                     const std::string& body,
                     const std::string& contentType = "application/json",
                     const std::map<std::string, std::string>& headers = {});

private:
    HttpResponse request(const std::string& method, const std::string& url,
                         const std::string& body, const std::string& ctype,
                         const std::map<std::string, std::string>& headers);
    std::string userAgent_ = "xcloud-switch/0.1";
    int timeoutSec_ = 30;
};

} // namespace xcloud
