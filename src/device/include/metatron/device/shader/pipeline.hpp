#pragma once
#include <metatron/device/shader/argument.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::shader {
    struct Pipeline final: stl::capsule<Pipeline> {
        struct Descriptor final {
            mut<command::Buffer> cmd;
            std::string_view name;
            std::vector<view<Argument>> args;
        };

        struct Impl;
        Pipeline(cref<Descriptor> desc) noexcept;
    };
}
