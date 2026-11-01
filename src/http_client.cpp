#include "xcloud/http_client.h"
#include <curl/curl.h>
#include <stdexcept>

namespace xcloud {

static size_t write_cb(void* ptr, size_t size, size_t nmemb, std::string* s) {
    size_t total = size * nmemb;
    s->append(static_cast<char*>(ptr), total);
    return total;
}

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

void HttpClient::setUserAgent(const std::string& ua) { userAgent_ = ua; }
void HttpClient::setTimeoutSec(int sec) { timeoutSec_ = sec; }

HttpResponse HttpClient::request(const std::string& method, const std::string& url,
                                const std::string& body, const std::string& ctype,
                                const std::map<std::string, std::string>& headers) {
    HttpResponse res;
    CURL* c = curl_easy_init();
    if (!c) { res.error = "curl_easy_init failed"; return res; }

    curl_easy_setopt(c, CURLOPT_URL, url.c_str());
    curl_easy_setopt(c, CURLOPT_USERAGENT, userAgent_.c_str());
    curl_easy_setopt(c, CURLOPT_TIMEOUT, timeoutSec_);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &res.body);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 2L);

    struct curl_slist* hlist = nullptr;
    for (const auto& kv : headers)
        hlist = curl_slist_append(hlist, (kv.first + ": " + kv.second).c_str());

    if (method == "POST") {
        curl_easy_setopt(c, CURLOPT_POST, 1L);
        if (!ctype.empty())
            hlist = curl_slist_append(hlist, ("Content-Type: " + ctype).c_str());
        curl_easy_setopt(c, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    } else if (method != "GET") {
        curl_easy_setopt(c, CURLOPT_CUSTOMREQUEST, method.c_str());
        if (!body.empty()) {
            curl_easy_setopt(c, CURLOPT_POSTFIELDS, body.data());
            curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
        }
    }

    if (hlist) curl_easy_setopt(c, CURLOPT_HTTPHEADER, hlist);

    CURLcode rc = curl_easy_perform(c);
    if (rc != CURLE_OK) res.error = curl_easy_strerror(rc);
    else curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &res.status);

    if (hlist) curl_slist_free_all(hlist);
    curl_easy_cleanup(c);
    return res;
}

HttpResponse HttpClient::get(const std::string& url,
                            const std::map<std::string, std::string>& headers) {
    return request("GET", url, "", "", headers);
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body,
                             const std::string& contentType,
                             const std::map<std::string, std::string>& headers) {
    return request("POST", url, body, contentType, headers);
}

} // namespace xcloud
