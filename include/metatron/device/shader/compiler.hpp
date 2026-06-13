#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <string_view>

namespace mtt::shader {
    struct Compiler final: stl::capsule<Compiler>, stl::singleton<Compiler> {
        struct Impl;
        Compiler() noexcept;
        auto static build(std::string_view dir, std::string_view out) noexcept -> void;
    };
}
