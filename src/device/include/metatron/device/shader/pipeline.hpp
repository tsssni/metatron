#pragma once
#include <metatron/core/stl/capsule.hpp>

namespace mtt::shader {
    struct Pipeline final: stl::capsule<Pipeline> {
        struct Impl;
        Pipeline(
            std::string_view shader,
            std::string_view entry
        ) noexcept;
    };
}
