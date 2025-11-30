#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/stack.hpp>
#include <functional>

namespace mtt::spectra {
    struct Color_Space final {
        enum struct Spectrum_Type: usize {
            albedo,
            unbounded,
            illuminant,
        };

        struct Transfer_Function final {
            std::function<f32(f32)> transfer;
            std::function<f32(f32)> linearize;
        };

        std::unordered_map<std::string, tag<Color_Space>> static color_spaces;

        fm33 from_XYZ;
        fm33 to_XYZ;
        tag<spectra::Spectrum> illuminant;
        tag<Transfer_Function> transfer_function;

        i32 table_res;
        f32 illuminant_Y_integral;
        buf<f32> scale;
        buf<f32> table;

        Color_Space(
            std::string_view name,
            cref<fv2> r, cref<fv2> g, cref<fv2> b,
            tag<spectra::Spectrum> illuminant,
            tag<Transfer_Function> transfer_function
        ) noexcept;
    };

    auto constexpr xyY_to_XYZ(cref<fv3> xyY) -> fv3 {
        auto [x, y, Y] = xyY;
        if (y < math::epsilon<f32>) return {0.f};
        else return {x * Y / y, Y, (1.f - x - y) * Y / y};
    };

    auto constexpr XYZ_to_xyY(cref<fv3> XYZ) -> fv3 {
        auto s = math::sum(XYZ);
        return {XYZ[0] / s, XYZ[1] / s, XYZ[1]};
    }
}
