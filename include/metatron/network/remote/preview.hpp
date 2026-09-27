#pragma once
#include <metatron/network/wired/address.hpp>
#include <metatron/resource/muldim/image.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::remote {
    struct Previewer final: stl::capsule<Previewer> {
        struct Impl;
        Previewer() noexcept = default;
        Previewer(wired::Address const& address, std::string_view name) noexcept;

        auto update(muldim::Image const& image, std::span<byte const> data = {}) noexcept -> void;
    };
}
