#include <metatron/render/spectra/rgb.hpp>
#include <metatron/render/spectra/spectrum.hpp>
#include <metatron/core/math/polynomial.hpp>

namespace mtt::render {

using Spectrum_Type = render::Color_Space::Spectrum_Type;

Rgb_Spectrum::Rgb_Spectrum(fv3 rgb, Spectrum_Type type, render::Color_Space const* color_space) noexcept {
    auto cs = color_space;
    illuminant = type == Spectrum_Type::illuminant ? color_space->illuminant : nullptr;

    s = 1.f;
    switch (type) {
    case Spectrum_Type::albedo:
        s = 1.f;
        break;
    case Spectrum_Type::unbounded:
        s = 2.f;
        break;
    case Spectrum_Type::illuminant:
        s = 2.f * core::max(rgb);
        break;
    }
    rgb = rgb / s;
    s /= type == Spectrum_Type::illuminant ? color_space->illuminant_Y_integral : 1.f;

    if (core::constant(rgb)) {
        c = fv3{(rgb[0] - 0.5f) / core::pow<1, 2>(rgb[0] * (1.f - rgb[0])), 0.f, 0.f};
        return;
    }

    auto maxc = core::maxi(rgb);
    auto z = rgb[maxc];
    auto x = rgb[(maxc + 1) % 3] * (cs->table_res - 1) / z;
    auto y = rgb[(maxc + 2) % 3] * (cs->table_res - 1) / z;

    // compute integer indices and offsets for coefficient interpolation
    auto scale = std::span<f32 const>{cs->scale.ptr, (usize)cs->table_res};
    auto bound = i32(std::ranges::lower_bound(scale, z) - std::begin(scale));
    auto xi = core::min((i32)x, cs->table_res - 2);
    auto yi = core::min((i32)y, cs->table_res - 2);
    auto zi = core::clamp(bound - 1, 0, cs->table_res - 2);

    auto dx = x - xi;
    auto dy = y - yi;
    auto dz = (z - cs->scale[zi]) / (cs->scale[zi + 1] - cs->scale[zi]);

    // trilinearly interpolate sigmoid polynomial coefficients
    for (auto i = 0; i < 3; ++i) {
        // define co lambda for looking up sigmoid polynomial coefficients
        auto co = [&](i32 dx, i32 dy, i32 dz) {
            auto idx = maxc * cs->table_res + zi + dz;
            idx = idx * cs->table_res + yi + dy;
            idx = idx * cs->table_res + xi + dx;
            return cs->table[idx * 3 + i];
        };

        c[2 - i] = core::lerp(
            core::lerp(
                core::lerp(co(0, 0, 0), co(1, 0, 0), dx),
                core::lerp(co(0, 1, 0), co(1, 1, 0), dx),
                dy
            ),
            core::lerp(
                core::lerp(co(0, 0, 1), co(1, 0, 1), dx),
                core::lerp(co(0, 1, 1), co(1, 1, 1), dx),
                dy
            ),
            dz
        );
    }
}

auto Rgb_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
    auto sigmoid = [](f32 x) -> f32 {
        if (core::isinf(x)) return x < 0.f ? 0.f : 1.f;
        return 0.5f + x / (2.f * core::pow<1, 2>(1.f + core::pow<2>(x)));
    };
    return s * sigmoid(core::polynomial(lambda, c)) * (illuminant ? (*illuminant)(lambda) : 1.f);
}

}
