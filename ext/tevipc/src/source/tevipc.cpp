#include <tevipc/tevipc.hpp>
#include <string_view>
#include <stdexcept>
#include <print>

namespace tevipc {
    IpcPacket::IpcPacket(const char* data, size_t length) {
        if (length <= 0) {
            throw std::runtime_error{"Cannot construct an IPC packet from no data."};
        }
        mPayload.assign(data, data + length);
    }

    void IpcPacket::setUpdateImage(
        std::string_view imageName,
        bool grabFocus,
        std::span<const IpcPacket::ChannelDesc> channelDescs,
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height,
        std::span<const float> stridedImageData
    ) {
        if (channelDescs.empty()) {
            throw std::runtime_error{"UpdateImage IPC packet must have a non-zero channel count."};
        }

        int32_t nChannels = (int32_t)channelDescs.size();
        std::vector<std::string> channelNames(nChannels);
        std::vector<int64_t> channelOffsets(nChannels);
        std::vector<int64_t> channelStrides(nChannels);

        for (int32_t i = 0; i < nChannels; ++i) {
            channelNames[i] = channelDescs[i].name;
            channelOffsets[i] = channelDescs[i].offset;
            channelStrides[i] = channelDescs[i].stride;
        }

        OStream payload{mPayload};
        payload << EType::UpdateImageV3;
        payload << grabFocus;
        payload << imageName;
        payload << nChannels;
        payload << channelNames;
        payload << x << y << width << height;
        payload << channelOffsets;
        payload << channelStrides;

        size_t nPixels = width * height;

        size_t stridedImageDataSize = 0;
        for (int32_t c = 0; c < nChannels; ++c) {
            stridedImageDataSize = std::max(stridedImageDataSize, (size_t)(channelOffsets[c] + (nPixels - 1) * channelStrides[c] + 1));
        }

        if (stridedImageData.size() != stridedImageDataSize) {
            throw std::runtime_error{std::format(
                "UpdateImage IPC packet's data size does not match specified dimensions, offset, and stride. (Expected: {})", stridedImageDataSize
            )};
        }

        payload << stridedImageData;
    }

    void IpcPacket::setCreateImage(
        std::string_view imageName, bool grabFocus, int32_t width, int32_t height, int32_t nChannels, std::span<const std::string> channelNames
    ) {
        if ((int32_t)channelNames.size() != nChannels) {
            throw std::runtime_error{"CreateImage IPC packet's channel names size does not match number of channels."};
        }

        OStream payload{mPayload};
        payload << EType::CreateImage;
        payload << grabFocus;
        payload << imageName;
        payload << width << height;
        payload << nChannels;
        payload << channelNames;
    }
}
