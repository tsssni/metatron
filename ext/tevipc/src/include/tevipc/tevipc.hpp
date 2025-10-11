#pragma once
#include <string>
#include <span>
#include <vector>
#include <cstdint>

namespace tevipc {
    // From https://github.com/Tom94/tev
    // By Thomas Muller

    struct IpcPacketUpdateImage {
        std::string imageName;
        bool grabFocus;
        int32_t nChannels;
        std::vector<std::string> channelNames;
        std::vector<int64_t> channelOffsets;
        std::vector<int64_t> channelStrides;
        int32_t x, y, width, height;
        std::vector<std::vector<float>> imageData; // One set of data per channel
    };

    struct IpcPacketCreateImage {
        std::string imageName;
        bool grabFocus;
        int32_t width, height;
        int32_t nChannels;
        std::vector<std::string> channelNames;
    };

    class IpcPacket {
    public:
        enum EType : char {
            OpenImage = 0,
            ReloadImage = 1,
            CloseImage = 2,
            UpdateImage = 3,
            CreateImage = 4,
            UpdateImageV2 = 5, // Adds multi-channel support
            UpdateImageV3 = 6, // Adds custom striding/offset support
            OpenImageV2 = 7,   // Explicit separation of image name and channel selector
            VectorGraphics = 8,
        };

        IpcPacket() = default;
        IpcPacket(const char* data, size_t length);

        const char* data() const { return mPayload.data(); }

        size_t size() const { return mPayload.size(); }

        EType type() const {
            // The first 4 bytes encode the message size.
            return (EType)mPayload[4];
        }

        struct ChannelDesc {
            std::string name;
            int64_t offset;
            int64_t stride;
        };

        void setUpdateImage(
            std::string_view imageName,
            bool grabFocus,
            std::span<const ChannelDesc> channelDescs,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height,
            std::span<const float> stridedImageData
        );
        void setCreateImage(
            std::string_view imageName, bool grabFocus, int32_t width, int32_t height, int32_t nChannels, std::span<const std::string> channelNames
        );

    private:
        std::vector<char> mPayload;

        class OStream {
        public:
            OStream(std::vector<char>& data) : mData{data} {
                // Reserve space for an integer denoting the size
                // of the packet.
                *this << (uint32_t)0;
            }

            template <typename T> OStream& operator<<(std::span<const T> var) {
                for (auto&& elem : var) {
                    *this << elem;
                }

                return *this;
            }

            template <typename T> OStream& operator<<(const std::vector<T>& var) {
                *this << std::span<const T>{var};
                return *this;
            }

            OStream& operator<<(std::string_view var) {
                for (auto&& character : var) {
                    *this << character;
                }

                *this << '\0';
                return *this;
            }

            OStream& operator<<(const std::string& var) {
                *this << std::string_view{var};
                return *this;
            }

            OStream& operator<<(bool var) {
                if (mData.size() < mIdx + 1) {
                    mData.resize(mIdx + 1);
                }

                mData[mIdx] = var ? 1 : 0;
                ++mIdx;
                updateSize();
                return *this;
            }

            template <typename T> OStream& operator<<(T var) {
                if (mData.size() < mIdx + sizeof(T)) {
                    mData.resize(mIdx + sizeof(T));
                }

                *(T*)&mData[mIdx] = var;
                mIdx += sizeof(T);
                updateSize();
                return *this;
            }

        private:
            void updateSize() { *((uint32_t*)mData.data()) = (uint32_t)mIdx; }

            std::vector<char>& mData;
            size_t mIdx = 0;
        };
    };

}
