#include <metatron/network/remote/preview.hpp>
#include <metatron/network/wired/tcp.hpp>
#include <tevipc/tevipc.hpp>

namespace mtt::remote {
    struct Previewer::Impl final {
        wired::Tcp_Socket socket;
        std::string name;
        std::array<std::string, 4> channels{"R", "G", "B", "A"};
        bool local{false};
        bool created{false};

        Impl(wired::Address const& address, std::string_view name) noexcept
        : socket(address), name(name), local(address.host.empty()) {}

        auto create(device::Texture const& image) noexcept -> void {
            if (created || local) return;

            auto packet = tevipc::IpcPacket{};
            packet.setCreateImage(
                name, false,
                image.width, image.height,
                image.channels, channels
            );
            created = socket.send({(byte*)packet.data(), packet.size()});
        }

        auto update(device::Texture&& image) noexcept -> void {
            create(image);
            if (!created || local || image.stride != 4) return;

            auto desc = std::array<tevipc::IpcPacket::ChannelDesc, 4>{};
            for (auto i = 0uz; i < 4uz; ++i) {
                desc[i].name = channels[i];
                desc[i].offset = i;
                desc[i].stride = image.channels;
            }

            auto packet = tevipc::IpcPacket{};
            packet.setUpdateImage(
                name, false, desc,
                0, 0, image.width, image.height,
                {(f32*)image.pixels.data(), image.pixels.size() / sizeof(f32)}
            );

            if (!socket.send({(byte*)packet.data(), packet.size()}))
                std::println("failed to send image to remote previewer");
        }
    };

    Previewer::Previewer(wired::Address const& address, std::string_view name) noexcept
    : stl::capsule<Previewer>(address, name) {}

    auto Previewer::update(device::Texture&& image) noexcept -> void {
        impl->update(std::move(image));
    }
}
