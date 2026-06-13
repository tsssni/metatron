#pragma once
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Pipeline final: stl::capsule<Pipeline> {
        std::vector<mut<Argument>> args;

        struct Descriptor final {
            std::string_view name;
            std::vector<mut<Argument>> args;
        };

        struct Impl;
        Pipeline(cref<Descriptor> desc) noexcept;
    };
}
