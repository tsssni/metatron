#include <metatron/render/spectra/color-space.hpp>
#include <metatron/render/spectra/spectrum.hpp>
#include <metatron/core/stl/filesystem.hpp>

namespace mtt::render {

Color_Space::Color_Space(
    std::string_view name,
    fm32 const& rgb, render::Discrete_Spectrum* i,
    std::array<render::Spectrum, 3> const& CIE_XYZ
) noexcept
: illuminant(i), illuminant_Y_integral(CIE_XYZ[1] | i) {
    auto to_XYZ = [&CIE_XYZ](render::Spectrum s) noexcept -> fv3 {
        return {CIE_XYZ[0] | s, CIE_XYZ[1] | s, CIE_XYZ[2] | s};
    };
    auto xyY_to_XYZ = [](fv3 const& xyY) noexcept -> fv3 {
        auto [x, y, Y] = xyY;
        return {x * Y / y, Y, (1.f - x - y) * Y / y};
    };
    auto XYZ_to_xyY = [](fv3 const& XYZ) noexcept -> fv3 {
        auto s = core::sum(XYZ);
        return {XYZ[0] / s, XYZ[1] / s, XYZ[1]};
    };

    // project color primaries and white point to Y=1
    auto w = XYZ_to_xyY(to_XYZ(illuminant));
    w = xyY_to_XYZ({w[0], w[1], 1.f});

    auto prims = core::transpose(fm33{
        xyY_to_XYZ({rgb[0], 1.f}),
        xyY_to_XYZ({rgb[1], 1.f}),
        xyY_to_XYZ({rgb[2], 1.f}),
    });
    auto inv_rgb = core::inverse(prims);
    auto c = inv_rgb | w;
    this->to_XYZ = prims | fm33{c[0], c[1], c[2]};
    from_XYZ = core::inverse(this->to_XYZ);

    auto path = "color-space/" + std::string{name} + ".coeff";
    auto coeff = core::filesystem::find(path);
    auto file = std::ifstream{coeff, std::ios::binary};

    auto header = std::string(4, '\0');
    if (!file.read(header.data(), 4) || header != "SPEC")
        core::abort("{} coefficient has wrong header", name);
    file.read((char*)&table_res, sizeof(i32));
    scale = table_res;
    table = table_res * table_res * table_res * 3 * 3;

    scale.map();
    table.map();
    file.read((char*)scale.ptr, (usize)table_res * sizeof(f32));
    file.read((char*)table.ptr, (usize)table_res * table_res * table_res * 3 * 3 * sizeof(f32));
    scale.unmap();
    table.unmap();
    file.close();
}

}
