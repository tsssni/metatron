#include <metatron/network/remote/preview.hpp>
#include <metatron/network/wired/packet.hpp>
#include <metatron/network/wired/tcp.hpp>

namespace mtt::remote {
    struct Previewer::contents final {
        wired::Tcp_Socket socket;
        std::string name;
        std::array<std::string_view, 4> channels{"R", "G", "B", "A"};
        bool created{false};

        contents(wired::Address const& address, std::string_view name) noexcept:
        socket(address), name(name) {}

        auto create(muldim::Image const& image) noexcept -> void {
            if (created) return;
            auto constexpr type = byte(4);
            auto packet = wired::Packet{};
            packet.emplace(0, type, false);
            packet.emplace(std::string_view{name});
            packet.emplace(u32(image.width), u32(image.height), u32(image.channels));
            packet.emplace(std::span{channels.data(), image.channels});
            *(u32*)packet.payload.data() = packet.payload.size();
            created = socket.send(packet.payload);
        }

        auto update(muldim::Image const& image, std::span<byte const> data) noexcept -> void {
            create(image);
            if (!created || image.stride != sizeof(f32)) return;
            auto offsets = std::vector<u64>(image.channels);
            auto strides = std::vector<u64>(image.channels, image.channels);
            for (auto i = 0; i < image.channels; ++i) offsets[i] = i;

            auto constexpr type = byte(6);
            auto packet = wired::Packet{};
            packet.emplace(0, type, false);
            packet.emplace(std::string_view{name});
            packet.emplace(u32(image.channels));
            packet.emplace(std::span{channels.data(), image.channels});
            packet.emplace(0, 0, u32(image.width), u32(image.height));
            packet.emplace(std::span{offsets});
            packet.emplace(std::span{strides});
            *(u32*)packet.payload.data() = packet.payload.size() + data.size();

            if (!socket.send(packet.payload, data)) stl::print("failed to preview image");
        }
    };

    Previewer::Previewer(wired::Address const& address, std::string_view name) noexcept:
    stl::shell<Previewer>(address, name) {}

    auto Previewer::update(muldim::Image const& image, std::span<byte const> data) noexcept -> void {
        impl->update(image, data.size() == 0 ? image.pixels.front() : data);
    }
}
