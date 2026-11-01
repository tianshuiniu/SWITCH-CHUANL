#pragma once
#include "xcloud/types.h"
#include <string>

namespace xcloud {

// 鉴权流程封装：
//   MSA(device-code) -> Xbox User Auth -> XSTS(gssv) -> xCloud(GSSV) 登录
// 返回 CloudSession 供后续串流信令使用。
class Auth {
public:
    // 走 device-code 流程，过程中会在控制台打印用户码与验证链接，并轮询直至用户登录。
    // 返回 MSA 访问/刷新令牌。
    static MsaTokens loginMsa();

    // user.auth.xboxlive.com：用 MSA 访问令牌换用户令牌（RPS ticket）。
    static XboxUserAuth authorizeUser(const std::string& msaAccessToken);

    // xsts.auth.xboxlive.com：用用户令牌换 XSTS（relying party = gssv）。
    static XboxXsts authorizeXsts(const std::string& userToken);

    // xhome.gssv-play-prod.xboxlive.com/v2/login/user：用 XSTS 登录 xCloud。
    static CloudSession loginCloud(const XboxXsts& xsts, const std::string& gamertag);

    // 一站式：device-code 登录并拿到 CloudSession。
    static CloudSession fullLogin();
};

// 工具：把字符串做 application/x-www-form-urlencoded 编码（用于表单体）。
std::string urlEncode(const std::string& in);

} // namespace xcloud
