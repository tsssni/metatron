#pragma once
#include <metatron/network/wired/address.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::wired {
    struct Tcp_Socket final: stl::capsule<Tcp_Socket> {
        struct Impl;
        Tcp_Socket() noexcept = default;
        Tcp_Socket(cref<Address> address) noexcept;

        auto send(std::span<byte const> data) noexcept -> bool;
        auto send(std::span<byte const> header, std::span<byte const> data) noexcept -> bool;
    };
}
