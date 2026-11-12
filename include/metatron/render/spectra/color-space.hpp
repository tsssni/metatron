#pragma once
#include <metatron/render/spectra/discrete.hpp>
#include <metatron/render/spectra/interaction.hpp>
#include <metatron/render/spectra/visible.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::render {

struct [[mtt::reflection(
    CIE_XYZ = {
        "/spectra/CIE-X",
        "/spectra/CIE-Y",
        "/spectra/CIE-Z"
    }
)]] Color_Space final {
    enum struct Spectrum_Type {
        albedo,
        unbounded,
        illuminant,
    };

    fm33 from_XYZ;
    fm33 to_XYZ;
    Discrete_Spectrum* illuminant;

    i32 table_res;
    f32 illuminant_Y_integral;
    buf<f32> scale;
    buf<f32> table;

    Color_Space(
        std::string_view path,
        fm32 const& rgb,
        Discrete_Spectrum* i,
        std::array<Visible_Spectrum*, 3> const& CIE_XYZ
    ) noexcept;
};

}
