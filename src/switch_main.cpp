// libnx (Switch) 入口：初始化网络与控制台，跑通 xCloud 登录 + 配置拉取，
// 并在屏幕上打印 device-code 登录所需信息。
//
// 注意：Switch 上无法弹出浏览器，device-code 流程的验证码与链接会打印在
// Switch 屏幕上，你需在手机/电脑浏览器中打开链接并输入验证码完成登录。
//
// 仅依赖 libnx + switch-curl + switch-mbedtls（devkitPro 提供），其余鉴权
// 逻辑与桌面端完全共用（src/auth.cpp / src/stream.cpp）。

#include <switch.h>
#include <iostream>
#include <string>
#include "xcloud/auth.h"
#include "xcloud/stream.h"
#include "xcloud/config.h"

int main(int argc, char** argv) {
    consoleInit();

    // 初始化 Switch 网络栈（switch-curl 依赖它）
    Result rc = socketInitializeDefault();
    if (R_FAILED(rc)) {
        std::cout << "[错误] socketInitializeDefault 失败: " << R_DESCRIPTION(rc) << "\n";
        std::cout << "请确认已连接 Wi-Fi。按 + 退出。\n";
        goto wait_exit;
    }

    std::cout << "=== xCloud 独立客户端 (Switch/libnx) ===\n";
    std::cout << "client_id = " << xcloud::kClientId << "\n\n";
    std::cout << "提示：登录验证码将显示在屏幕，请到手机/电脑浏览器操作。\n\n";

    try {
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
        std::cout << "\n[错误] " << e.what() << "\n";
    }

wait_exit:
    std::cout << "\n按 + 退出\n";
    while (appletMainLoop()) {
        hidScanInput();
        u64 k = hidKeysDown(CONTROLLER_P1_AUTO);
        if (k & KEY_PLUS) break;
    }

    socketExit();
    consoleExit();
    return 0;
}
