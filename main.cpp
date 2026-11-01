#include "xcloud/auth.h"
#include "xcloud/stream.h"
#include "xcloud/config.h"
#include <iostream>

// 桌面端验证用 CLI：完成登录并打印 xCloud 会话与配置。
// Switch 端只需把 HttpClient 换成 libnx 后端，其余逻辑复用。
int main() {
    try {
        std::cout << "=== xCloud 独立客户端（研究/互操作）===\n";
        std::cout << "client_id = " << xcloud::kClientId << "\n\n";

        xcloud::CloudSession s = xcloud::Auth::fullLogin();

        std::cout << "\n[会话] serverId : " << s.serverId << "\n";
        std::cout << "[会话] baseUri  : " << s.baseUri << "\n";
        std::cout << "[会话] gsToken  : " << s.gsToken.substr(0, 16) << "...(已截断)\n";
        std::cout << "[会话] expiresAt: " << s.expiresAt << "\n";
        if (!s.gamertag.empty())
            std::cout << "[会话] gamertag : " << s.gamertag << "\n";

        std::cout << "\n[配置] 拉取串流会话配置...\n";
        auto cfg = xcloud::Stream::getConfiguration(s);
        std::cout << cfg.dump(2) << "\n";

        const auto& reg = xcloud::Stream::defaultRegion();
        std::cout << "[区域] 默认 " << reg.name << " (" << reg.label
                  << ") 边缘 " << reg.ip << "\n";
    } catch (const std::exception& e) {
        std::cerr << "\n[错误] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
