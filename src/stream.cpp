#include "xcloud/stream.h"
#include "xcloud/config.h"
#include "xcloud/http_client.h"
#include <stdexcept>

using json = nlohmann::json;

namespace xcloud {

nlohmann::json Stream::getConfiguration(const CloudSession& s) {
    HttpClient http;
    http.setUserAgent(kUserAgent);
    std::map<std::string, std::string> hdr = {
        {"Authorization", "Bearer " + s.gsToken}
    };
    std::string url = s.baseUri + "/streaming/systemUi/configuration";
    auto r = http.get(url, hdr);
    if (!r.error.empty() || r.status != 200)
        throw std::runtime_error("getConfiguration failed: " + r.error +
                                 " status=" + std::to_string(r.status) + " body=" + r.body);
    return json::parse(r.body);
}

const Region& Stream::pickRegion(const std::string& name) {
    for (const auto& r : kRegions)
        if (r.name == name) return r;
    throw std::runtime_error("未知区域: " + name);
}

const Region& Stream::defaultRegion() {
    // 默认东南亚（en），如需中文体验可改 AUSTRALIAEAST
    return kRegions[5];
}

std::string Stream::buildSdpOfferTemplate(const std::string& ufrag,
                                          const std::string& pwd,
                                          const std::string& fingerprint) {
    // 参考 xCloud 协商格式：H.265 视频 + Opus 音频 + transport-wide-cc 拥塞控制
    std::string sdp;
    sdp += "v=0\r\n";
    sdp += "o=- 0 0 IN IP4 0.0.0.0\r\n";
    sdp += "s=xfc\r\n";
    sdp += "t=0 0\r\n";
    sdp += "a=ice-options:trickle\r\n";
    sdp += "a=fingerprint:sha-256 " + fingerprint + "\r\n";
    sdp += "m=video 9 UDP/TLS/RTP/SAVPF 96 97\r\n";
    sdp += "c=IN IP4 0.0.0.0\r\n";
    sdp += "a=rtcp-mux\r\n";
    sdp += "a=ice-ufrag:" + ufrag + "\r\n";
    sdp += "a=ice-pwd:" + pwd + "\r\n";
    sdp += "a=setup:actpass\r\n";
    sdp += "a=mid:0\r\n";
    sdp += "a=extmap:1 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\r\n";
    sdp += "a=rtpmap:96 H265/90000\r\n";   // xCloud 亦支持 H264 / AV1
    sdp += "a=rtpmap:97 opus/48000/2\r\n";
    sdp += "a=fmtp:97 minptime=10;maxaveragebitrate=96000;stereo=1;sprop-stereo=1;useinbandfec=1\r\n";
    sdp += "a=sendrecv\r\n";
    return sdp;
}

} // namespace xcloud
