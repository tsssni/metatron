#pragma once
#include <metatron/core/stl/protocol.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::muldim {
    struct Grid final {
        union {
            struct {
                usize width;
                usize height;
                usize depth;
            };
            uzv3 size;
        };
        std::vector<f32> cells;

        auto operator[](usize x, usize y, usize z) noexcept -> f32&;
        auto operator[](usize x, usize y, usize z) const noexcept -> f32;
        auto operator()(cref<fv3> uvw) const -> fv4;
    };
}

namespace mtt::muldim::proxy {
    struct Grid: stl::proxy<Grid, muldim::Grid> {
        using proxy::proxy;
        auto operator()(cref<fv3> uvw) const noexcept -> fv4 { return (*idx)(uvw); }
        auto operator[](usize x, usize y, usize z) noexcept -> f32& { return (*idx)[x, y, z]; }
        auto operator[](usize x, usize y, usize z) const noexcept -> f32 { return (*idx)[x, y, z]; }
    };
}
