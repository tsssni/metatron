#pragma once
#include <metatron/network/wired/address.hpp>
#include <metatron/resource/muldim/image.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::remote {
    struct Previewer final: stl::capsule<Previewer> {
        struct Impl;
        Previewer() noexcept = default;
        Previewer(cref<wired::Address> address, std::string_view name) noexcept;

        auto update(cref<muldim::Image> image, std::span<byte const> data = {}) noexcept -> void;
    };
}
