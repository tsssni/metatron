#pragma once
#include <metatron/resource/color/Rec709.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::color {
    struct Transfer_Function final: stl::polynomial<Transfer_Function
    , Rec709_Transfer_Function> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto transfer(f32 x) const noexcept -> f32 {
            return visit([x](auto* p) noexcept -> f32 { return p->transfer(x); });
        }

        auto linearize(f32 x) const noexcept -> f32 {
            return visit([x](auto* p) noexcept -> f32 { return p->linearize(x); });
        }
    };
}
