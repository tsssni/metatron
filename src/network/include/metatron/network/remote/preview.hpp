#pragma once
#include <metatron/network/wired/address.hpp>
#include <metatron/resource/opaque/image.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::remote {
    struct Previewer final: stl::capsule<Previewer> {
        struct Impl;
        Previewer() noexcept = default;
        Previewer(cref<wired::Address> address, std::string_view name) noexcept;

        auto update(cref<opaque::Image> image) noexcept -> void;
    };
}
