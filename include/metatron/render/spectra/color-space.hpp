#pragma once
#include <metatron/render/spectra/discrete.hpp>
#include <metatron/render/spectra/interaction.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::render {

struct Color_Space final {
    enum struct Spectrum_Type {
        albedo,
        unbounded,
        illuminant,
    };

    fm33 from_XYZ;
    fm33 to_XYZ;
    render::Discrete_Spectrum* illuminant;

    i32 table_res;
    f32 illuminant_Y_integral;
    buf<f32> scale;
    buf<f32> table;

    Color_Space(
        std::string_view name,
        fm32 const& rgb,
        render::Discrete_Spectrum* i,
        std::array<render::Spectrum, 3> const& CIE_XYZ
    ) noexcept;
};

}
