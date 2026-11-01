#pragma once
#include "xcloud/types.h"
#include "xcloud/config.h"
#include <nlohmann/json.hpp>
#include <string>

namespace xcloud {

// xCloud 串流信令 / 配置接口层（WebRTC 媒体传输为第二阶段，此处只做 REST 与会话构造）
class Stream {
public:
    // 拉取串流会话配置（分辨率档位、输入能力、特性开关等）
    static nlohmann::json getConfiguration(const CloudSession& s);

    // 选择一个区域（返回引用；不拷贝）
    static const Region& pickRegion(const std::string& name);

    // 默认区域（东南亚/澳大利亚对中文用户友好，按需要改）
    static const Region& defaultRegion();

    // 构造一个 WebRTC SDP offer 模板（H.265 视频 + Opus 音频 + transport-cc）。
    // 注意：真正的 offer 应由 WebRTC 栈（如 libdatachannel）在连接时生成，
    // 这里仅提供符合 xCloud 协商格式的参考模板，供第二阶段对接与联调。
    static std::string buildSdpOfferTemplate(const std::string& ufrag,
                                             const std::string& pwd,
                                             const std::string& fingerprint);
};

} // namespace xcloud
