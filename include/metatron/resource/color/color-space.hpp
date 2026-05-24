#pragma once
#include <metatron/resource/color/transfer-function.hpp>
#include <metatron/resource/spectra/interaction.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/protocol.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/stack.hpp>
#include <functional>

namespace mtt::color {
    struct Color_Space final {
        enum struct Spectrum_Type {
            albedo,
            unbounded,
            illuminant,
        };

        fm33 from_XYZ;
        fm33 to_XYZ;
        u32 illuminant;
        Transfer_Function transfer_function;

        i32 table_res;
        f32 illuminant_Y_integral;
        buf<f32> scale;
        buf<f32> table;

        Color_Space(
            std::string_view name,
            cref<fv2> r, cref<fv2> g, cref<fv2> b, u32 i,
            Transfer_Function transfer_function
        ) noexcept;
    };

    auto constexpr xyY_to_XYZ(cref<fv3> xyY) -> fv3 {
        auto [x, y, Y] = xyY;
        return {x * Y / y, Y, (1.f - x - y) * Y / y};
    };

    auto constexpr XYZ_to_xyY(cref<fv3> XYZ) -> fv3 {
        auto s = math::sum(XYZ);
        return {XYZ[0] / s, XYZ[1] / s, XYZ[1]};
    }
}

namespace mtt::color::proxy {
    struct Color_Space: stl::proxy<Color_Space, color::Color_Space> {
        using proxy::proxy;
        auto static init() noexcept -> void;
    };
}
