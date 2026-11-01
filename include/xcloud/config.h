#pragma once
#include <string>
#include <vector>
#include <map>

// 独立实现的 xCloud (Xbox Cloud Gaming) 串流客户端 —— 鉴权 + REST 接口层
// 仅用于研究 / 教育 / 互操作性，使用你自己的微软账号。
// 所有端点均来自对 xgo.nro 的静态分析核实，属公开 xCloud 协议。

namespace xcloud {

// 公开的个人客户端 ID（xCloud 网页版使用的同一个），非秘密封钥
constexpr const char* kClientId = "000000004c20a908";
constexpr const char* kScope    = "service::user.auth.xboxlive.com::MBI_SSL";
constexpr const char* kUserAgent = "xcloud-switch/0.1 (research)";

// ---- 微软 / Xbox Live 端点 ----
constexpr const char* kMsDeviceCodeUrl   = "https://login.microsoftonline.com/consumers/oauth2/v2.0/devicecode";
constexpr const char* kMsTokenUrl        = "https://login.microsoftonline.com/consumers/oauth2/v2.0/token";
constexpr const char* kXblUserAuthUrl    = "https://user.auth.xboxlive.com/user/authenticate";
constexpr const char* kXstsAuthUrl       = "https://xsts.auth.xboxlive.com/xsts/authorize";
constexpr const char* kXcloudLoginUrl    = "https://xhome.gssv-play-prod.xboxlive.com/v2/login/user";

// ---- 区域边缘节点（来自 xgo config.json 的 region_ips）----
struct Region {
    std::string name;   // EASTUS 等
    std::string label;  // 中文显示名
    std::string ip;     // WebRTC 媒体边缘 IP
    std::string lang;   // 默认语言
};
inline const std::vector<Region> kRegions = {
    {"EASTUS",         "美国东部",   "203.41.44.20", "en"},
    {"WESTUS",         "美国西部",   "68.224.57.1",  "en"},
    {"WESTEUROPE",     "西欧",       "52.165.177.1", "en"},
    {"JAPANEAST",      "日本东部",   "20.43.132.1",  "ja"},
    {"KOREACENTRAL",   "韩国中部",   "20.200.225.1", "ko"},
    {"SOUTHEASTASIA",  "东南亚",     "20.73.130.1",  "en"},
    {"AUSTRALIAEAST",  "澳大利亚东部","20.195.85.1",  "zh"},
};

// ---- WebRTC 连接辅助（来自 config.json）----
inline const std::vector<std::string> kStunServers = {
    "stun:stun.miwifi.com:3478",
    "stun:stun.chat.bilibili.com:3478",
};
inline const std::vector<std::string> kDohServers = {
    "https://dns.alidns.com/resolve",
    "https://1.12.12.12/dns-query",
    "https://8.8.8.8/resolve",
};
inline const std::vector<int> kTeredoPorts = {9002};

} // namespace xcloud
