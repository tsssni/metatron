#pragma once
#include <metatron/network/wired/address.hpp>
#include <metatron/core/stl/shell.hpp>

namespace mtt::wired {
    struct Tcp_Socket final: stl::shell<Tcp_Socket> {
        struct contents;
        Tcp_Socket() noexcept = default;
        Tcp_Socket(Address const& address) noexcept;

        auto send(std::span<byte const> data) noexcept -> bool;
        auto send(std::span<byte const> header, std::span<byte const> data) noexcept -> bool;
    };
}
