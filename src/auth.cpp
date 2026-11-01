#include "xcloud/auth.h"
#include "xcloud/config.h"
#include "xcloud/http_client.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

using json = nlohmann::json;

namespace xcloud {

std::string urlEncode(const std::string& in) {
    CURL* c = curl_easy_init();
    char* out = curl_easy_escape(c, in.c_str(), static_cast<int>(in.size()));
    std::string r(out ? out : "");
    if (out) curl_free(out);
    if (c) curl_easy_cleanup(c);
    return r;
}

static std::string makeUuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> d(0, 15);
    static const char* h = "0123456789abcdef";
    std::string s(36, '0');
    // xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    int pos[] = {8,13,18,23};
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) { s[i] = '-'; continue; }
        if (i == 14) { s[i] = '4'; continue; }
        if (i == 19) { s[i] = h[(d(gen) & 0x3) | 0x8]; continue; }
        s[i] = h[d(gen)];
    }
    return s;
}

MsaTokens Auth::loginMsa() {
    HttpClient http;
    http.setUserAgent(kUserAgent);

    std::string dcBody = "client_id=" + std::string(kClientId) +
                         "&scope=" + urlEncode(kScope);
    auto r = http.post(kMsDeviceCodeUrl, dcBody, "application/x-www-form-urlencoded");
    if (!r.error.empty() || r.status != 200)
        throw std::runtime_error("devicecode failed: " + r.error + " status=" + std::to_string(r.status) + " body=" + r.body);

    auto j = json::parse(r.body);
    std::string userCode  = j.value("user_code", "");
    std::string devCode   = j.value("device_code", "");
    std::string verifyUri = j.value("verification_uri", j.value("verification_uri_complete", ""));
    int interval = j.value("interval", 5);
    int expires  = j.value("expires_in", 900);

    std::cout << "\n[MSA] 请在浏览器打开: " << verifyUri << "\n";
    std::cout << "[MSA] 输入用户码: " << userCode << "\n";
    std::cout << "[MSA] 等待登录（超时 " << expires << "s）...\n";

    std::string tokenBody = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Adevice_code"
                            "&client_id=" + std::string(kClientId) +
                            "&device_code=" + urlEncode(devCode);

    int waited = 0;
    while (waited < expires) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        waited += interval;
        auto t = http.post(kMsTokenUrl, tokenBody, "application/x-www-form-urlencoded");
        if (!t.error.empty()) throw std::runtime_error("token poll error: " + t.error);
        if (t.status == 200) {
            auto tk = json::parse(t.body);
            MsaTokens m;
            m.accessToken  = tk.value("access_token", "");
            m.refreshToken = tk.value("refresh_token", "");
            m.expiresIn    = tk.value("expires_in", 0);
            m.tokenType    = tk.value("token_type", "Bearer");
            std::cout << "[MSA] 登录成功，拿到访问令牌。\n";
            return m;
        }
        // 否则继续轮询（authorization_pending / slow_down）
    }
    throw std::runtime_error("device-code 登录超时");
}

XboxUserAuth Auth::authorizeUser(const std::string& msaAccessToken) {
    HttpClient http;
    json body = {
        {"Properties", {
            {"AuthMethod", "RPS"},
            {"SiteName", "user.auth.xboxlive.com"},
            {"RpsTicket", "d=" + msaAccessToken}
        }},
        {"RelyingParty", "http://auth.xboxlive.com"},
        {"TokenType", "JWT"}
    };
    auto r = http.post(kXblUserAuthUrl, body.dump(), "application/json");
    if (!r.error.empty() || r.status != 200)
        throw std::runtime_error("user auth failed: " + r.error + " status=" + std::to_string(r.status) + " body=" + r.body);
    auto j = json::parse(r.body);
    XboxUserAuth u;
    u.token = j.value("Token", "");
    try { u.uhs = j["DisplayClaims"]["xui"][0]["uhs"].get<std::string>(); } catch (...) {}
    return u;
}

XboxXsts Auth::authorizeXsts(const std::string& userToken) {
    HttpClient http;
    json body = {
        {"Properties", {
            {"SandboxId", "RETAIL"},
            {"UserTokens", json::array({userToken})},
            {"RelyingParty", "http://xboxlive.com"}
        }},
        {"RelyingParty", "http://gssv.xboxlive.com/"},
        {"TokenType", "JWT"}
    };
    auto r = http.post(kXstsAuthUrl, body.dump(), "application/json");
    if (!r.error.empty() || r.status != 200)
        throw std::runtime_error("xsts failed: " + r.error + " status=" + std::to_string(r.status) + " body=" + r.body);
    auto j = json::parse(r.body);
    XboxXsts x;
    x.token = j.value("Token", "");
    try {
        auto xui = j["DisplayClaims"]["xui"][0];
        x.uhs = xui.value("uhs", "");
        x.gti = xui.value("gti", "");
        x.gsp = xui.value("gsp", "");
        x.gamertag = xui.value("gtg", xui.value("gamertag", ""));
    } catch (...) {}
    return x;
}

CloudSession Auth::loginCloud(const XboxXsts& xsts, const std::string& gamertag) {
    HttpClient http;
    http.setUserAgent(kUserAgent);

    json props = {
        {"UserTokens", json::array({xsts.token})},
        {"DeviceTokens", json::array()},
        {"TitleTokens", json::array()},
        {"SandboxId", "RETAIL"},
        {"UseCompressedMessageFormat", true},
        {"QueryParameters", {
            {"xal_region", "USA"},
            {"xal_showstreamingstatus", "true"},
            {"knownViewerCount", "1"}
        }}
    };
    if (!xsts.gti.empty()) props["OptionalDisplayClaims"]["gti"] = xsts.gti;
    if (!xsts.gsp.empty()) props["OptionalDisplayClaims"]["gsp"] = xsts.gsp;

    json body = {
        {"serverId", makeUuid()},
        {"RelyingParty", "http://gssv.xboxlive.com/"},
        {"TokenType", "Bearer"},
        {"Properties", props}
    };

    std::map<std::string, std::string> hdr = {
        {"Authorization", "Bearer " + xsts.token}
    };
    auto r = http.post(kXcloudLoginUrl, body.dump(), "application/json", hdr);
    if (!r.error.empty() || (r.status != 200 && r.status != 201))
        throw std::runtime_error("xcloud login failed: " + r.error + " status=" + std::to_string(r.status) + " body=" + r.body);
    auto j = json::parse(r.body);
    CloudSession s;
    s.serverId  = j.value("serverId", "");
    s.gsToken   = j.value("gsToken", "");
    s.baseUri   = j.value("baseUri", "");
    s.expiresAt = j.value("expiresAt", "");
    s.xstsToken = xsts.token;
    s.gamertag  = gamertag.empty() ? xsts.gamertag : gamertag;
    return s;
}

CloudSession Auth::fullLogin() {
    MsaTokens msa = loginMsa();
    XboxUserAuth u = authorizeUser(msa.accessToken);
    XboxXsts x = authorizeXsts(u.token);
    if (u.uhs != x.uhs)
        throw std::runtime_error("uhs 不匹配：user.auth 与 xsts 的用户哈希不一致");
    return loginCloud(x, x.gamertag);
}

} // namespace xcloud
