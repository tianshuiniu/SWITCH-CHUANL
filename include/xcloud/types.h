#pragma once
#include <string>

namespace xcloud {

// 微软账号 OAuth2 令牌（device-code 流程）
struct MsaTokens {
    std::string accessToken;
    std::string refreshToken;
    long expiresIn = 0;
    std::string tokenType = "Bearer";
};

// Xbox Live 用户令牌（user.auth.xboxlive.com）
struct XboxUserAuth {
    std::string token;  // "u=..."
    std::string uhs;    // user hash，需与 XSTS 的 uhs 一致
};

// Xbox Live XSTS 令牌（xsts.auth.xboxlive.com，relying party = gssv）
struct XboxXsts {
    std::string token;  // "t=..."
    std::string uhs;
    std::string gti;    // 来自 DisplayClaims，GSSV 登录需用
    std::string gsp;    // 来自 DisplayClaims
    std::string gamertag;
};

// xCloud (GSSV) 登录会话
struct CloudSession {
    std::string serverId;
    std::string gsToken;
    std::string baseUri;     // 形如 https://<region>.gssv-play-prod.xboxlive.com
    std::string expiresAt;
    std::string xstsToken;   // 原始 XSTS，WebRTC 连接时仍要用
    std::string gamertag;
};

} // namespace xcloud
