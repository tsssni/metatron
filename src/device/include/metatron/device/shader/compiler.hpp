#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::shader {
    struct Compiler final: stl::capsule<Compiler>, stl::singleton<Compiler> {
        struct Impl;
        Compiler() noexcept;
        auto build(std::string_view dir, std::string_view out) noexcept -> void;
    };
}
