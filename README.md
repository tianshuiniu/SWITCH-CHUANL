# xcloud-client（研究 / 互操作）

一个**独立实现**的 Xbox Cloud Gaming（xCloud）串流客户端**鉴权 + REST 接口层**，用 C++ 编写，目标是 eventual 移植到 Nintendo Switch（libnx）。

> ⚠️ 仅用于研究 / 教育 / 互操作性，使用你**自己的**微软账号。
> 用非官方客户端连接 xCloud 违反微软 xCloud 设备条款，风险自负。
> 本仓库**不**包含任何破解、激活绕过或第三方闭源代码。

---

## 已实现（第一阶段：鉴权 + 接口层）

```
MSA(device-code) ──▶ Xbox User Auth ──▶ XSTS(gssv) ──▶ xCloud(GSSV) 登录 ──▶ 会话配置
```

- `client_id = 000000004c20a908`（xCloud 网页版公开使用的同一个，非秘密封钥）
- device-code 登录（控制台打印用户码 + 验证链接，自动轮询）
- user.auth.xboxlive.com → xsts.auth.xboxlive.com（relying party = `http://gssv.xboxlive.com/`）
- `xhome.gssv-play-prod.xboxlive.com/v2/login/user` 登录，得到 `serverId / gsToken / baseUri / expiresAt`
- 拉取 `/streaming/systemUi/configuration`
- 区域表、STUN / DoH 配置（来自对 xgo.nro 的静态分析核实）
- WebRTC SDP offer **模板**（第二阶段对接 libdatachannel 时生成真实 offer）

## 目录

```
include/xcloud/   config.h  types.h  http_client.h  auth.h  stream.h
src/              http_client.cpp  auth.cpp  stream.cpp  switch_main.cpp(libnx 入口)
main.cpp          CLI 验证程序（桌面）
CMakeLists.txt    桌面构建
Makefile          Switch(libnx) 构建 → xcloud_switch.nro
third_party/nlohmann/json.hpp   vendored nlohmann/json 单头文件（Switch 免装包）
icon.jpg          256×256 NRO 图标
```

## 桌面端构建与验证

依赖：C++17、libcurl、nlohmann-json（vcpkg / apt / brew 均可）。

```bash
cmake -B build -S .
cmake --build build
./build/xcloud_cli
```

运行后会打印微软登录链接与用户码 → 浏览器登录 → 自动完成 Xbox 登录 → 打印 `baseUri / gsToken / 配置`。
这一步**不需要 Switch**，用来确认鉴权链路在你账号上可用。

## Switch（libnx）构建与上机

依赖（devkitPro 的 pacman 安装）：

```bash
# Windows 用 devkitPro 图形安装器勾选 "Switch Development"；
# Linux/macOS：
sudo (dkp-)pacman -S switch-dev switch-curl switch-mbedtls
# switch-dev 已含 libnx 与 devkitA64 工具链
```

构建（产物 `xcloud_switch.nro`）：

```bash
export DEVKITPRO=/opt/devkitpro   # Windows 安装器已自动设置
make
# 产出 xcloud_switch.nro
```

上机方式（任选其一）：
- 拷到 SD 卡 `switch/` 目录，从 hbmenu 启动；
- 或用 `nxlink`（hbmenu 里按 Y 开 NetLoader）：`nxlink -a <Switch_IP> xcloud_switch.nro`。

运行后，屏幕会打印 **device-code 登录链接与用户码**（Switch 上无浏览器），
用手机/电脑浏览器打开链接、输入验证码完成微软登录，程序自动完成 Xbox 登录并打印
`baseUri / gsToken / 配置`。按 `+` 退出。

> 注意：`HttpClient` 直接复用 libcurl（`switch-curl` 与桌面 API 兼容），
> 因此鉴权 / 信令逻辑与桌面端**完全一致**，无需为 Switch 重写网络层。
> `switch_main.cpp` 只负责 libnx 初始化（网络栈、控制台）与调用同一套 `Auth` / `Stream`。

### Switch 上机排错（已知坑）

- **TLS 证书**：`switch-curl`（mbedtls 后端）默认**没有 CA 证书包**，直连 `login.live.com` 会报
  SSL 校验失败。解决：把一份 `ca-bundle.pem` 放到 SD 卡，`HttpClient` 里
  `curl_easy_setopt(c, CURLOPT_CAINFO, "sdmc:/switch/xcloud/ca-bundle.pem")`（或在 `switch_main.cpp`
  启动后 `setenv("CURL_CA_BUNDLE", "sdmc:/switch/xcloud/ca-bundle.pem", 1)`）。
  也可暂时 `CURLOPT_SSL_VERIFYPEER=0L` 仅用于联调（不建议发布）。
- **网络初始化**：必须 `socketInitializeDefault()` 后才能用 curl，已在 `switch_main.cpp` 处理。
- **device-code 登录**：Switch 无浏览器，验证码与链接打印在屏幕，需手机/电脑配合完成登录。

## 第二阶段（未实现，需移植库）

| 模块 | 方案 | 在 Makefile 的 LIBS 追加 |
|---|---|---|
| WebRTC 信令 + ICE | 移植 **libdatachannel** | `-ldatachannel` |
| 视频解码 H.265/H.264/AV1 | **FFmpeg** (`libavcodec`)，经硬件或软件解码 | `-lavformat -lavcodec -lswscale -lavutil` |
| 音频解码 Opus | FFmpeg / libopus | `-lopus` |
| 渲染 / 输入回传 | Switch 图形栈 + HID，经 WebRTC DataChannel 回传手柄输入 | `-lglad -lEGL`（视渲染方案） |

## 第二阶段（未实现，需移植库）

| 模块 | 方案 |
|---|---|
| WebRTC 信令 + ICE | 移植 **libdatachannel**，用 `buildSdpOfferTemplate()` 的格式做协商 |
| 视频解码 H.265/H.264/AV1 | **FFmpeg** (`libavcodec`)，经硬件或软件解码 |
| 音频解码 Opus | FFmpeg / libopus |
| 渲染 / 输入回传 | Switch 图形栈 + HID，经 WebRTC DataChannel 回传手柄输入 |

## 已知不确定项（需真机/真账号联调）

- GSSV 登录请求体的 `QueryParameters` / `OptionalDisplayClaims` 字段随 xCloud 版本可能微调；
  以 `xsts.DisplayClaims.xui[0]` 中的 `gti` / `gsp` 为准。
- 部分字段（如 gamertag）可能来自额外 profile 调用，已做容错。
- 本代码**未经实机运行验证**，仅保证结构正确、可在桌面编译。
