#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/vector.hpp>
#include <functional>

namespace mtt::color {
    struct Transfer_Function final {
        std::function<f32(f32)> transfer;
        std::function<f32(f32)> linearize;
    };

    struct Color_Space final {
        enum struct Spectrum_Type: usize {
            albedo,
            unbounded,
            illuminant,
        };

        std::unordered_map<std::string, proxy<Color_Space>> static color_spaces;

        math::Matrix<f32, 3, 3> from_XYZ;
        math::Matrix<f32, 3, 3> to_XYZ;
        proxy<spectra::Spectrum> illuminant;
        proxy<Transfer_Function> transfer_function;

        i32 table_res;
        f32 illuminant_Y_integral;
        std::vector<f32> scale;
        std::vector<f32> table;

        Color_Space(
            std::string_view name,
            math::Vector<f32, 2> const& r,
            math::Vector<f32, 2> const& g,
            math::Vector<f32, 2> const& b,
            proxy<spectra::Spectrum> illuminant,
            proxy<Transfer_Function> transfer_function
        );
    };

    auto constexpr xyY_to_XYZ(math::Vector<f32, 3> const& xyY) -> math::Vector<f32, 3> {
        auto [x, y, Y] = xyY;
        if (y < math::epsilon<f32>) return {0.f};
        else return {x * Y / y, Y, (1.f - x - y) * Y / y};
    };

    auto constexpr XYZ_to_xyY(math::Vector<f32, 3> const& XYZ) -> math::Vector<f32, 3> {
        auto s = math::sum(XYZ);
        return {XYZ[0] / s, XYZ[1] / s, XYZ[1]};
    }
}
