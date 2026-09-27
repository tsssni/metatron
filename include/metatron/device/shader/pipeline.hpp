#pragma once
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Pipeline final: stl::capsule<Pipeline> {
        std::vector<Argument*> args;

        struct Descriptor final {
            std::string_view name;
            std::vector<Argument*> args;
        };

        struct Impl;
        Pipeline(Descriptor const& desc) noexcept;
    };
}
