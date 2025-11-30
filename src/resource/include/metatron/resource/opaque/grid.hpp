#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::opaque {
    struct Grid final {
        union {
            struct {
                usize width;
                usize height;
                usize depth;
            };
            uzv3 size;
        };
        buf<f32> cells;

        auto operator[](usize x, usize y, usize z) noexcept -> f32&;
        auto operator[](usize x, usize y, usize z) const noexcept -> f32;
        auto operator()(cref<fv3> uvw) const -> fv4;
    };
}
