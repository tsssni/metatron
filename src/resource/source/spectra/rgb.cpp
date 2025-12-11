#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/polynomial.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::spectra {
    using Spectrum_Type = Color_Space::Spectrum_Type;

    Rgb_Spectrum::Rgb_Spectrum(cref<Descriptor> desc) noexcept {
        auto rgb = desc.c;
        auto cs = desc.color_space;
        illuminant = desc.type == Spectrum_Type::illuminant
        ? desc.color_space->illuminant : tag<Spectrum>{};

        s = 1.f;
        switch (desc.type) {
        case Spectrum_Type::albedo:
            s = 1.f;
            break;
        case Spectrum_Type::unbounded:
            s = 2.f;
            break;
        case Spectrum_Type::illuminant:
            s = 2.f * math::max(rgb[0], math::max(rgb[1], rgb[2]));
            break;
        }
        rgb = rgb / s;
        s /= desc.type == Spectrum_Type::illuminant
        ? desc.color_space->illuminant_Y_integral : 1.f;

        if (rgb[0] == rgb[1] && rgb[1] == rgb[2]) {
            this->c = fv3{
                (rgb[0] - 0.5f) / math::sqrt(rgb[0] * (1.f - rgb[0])),
                0.f, 0.f,
            };
            return;
        }

        auto maxc = math::maxi(rgb);
        auto z = rgb[maxc];
        auto x = rgb[(maxc + 1) % 3] * (cs->table_res - 1) / z;
        auto y = rgb[(maxc + 2) % 3] * (cs->table_res - 1) / z;

        // compute integer indices and offsets for coefficient interpolation
        auto scale = std::span<f32>{cs->scale};
        auto xi = math::min((i32)x, cs->table_res - 2);
        auto yi = math::min((i32)y, cs->table_res - 2);
        auto zi = math::clamp(
            i32(std::ranges::lower_bound(scale, z) - std::begin(scale)) - 1,
            0, cs->table_res - 2
        );

        auto dx = x - xi;
        auto dy = y - yi;
        auto dz = (z - cs->scale[zi]) / (cs->scale[zi + 1] - cs->scale[zi]);

        // trilinearly interpolate sigmoid polynomial coefficients
        for (auto i = 0; i < 3; ++i) {
            // define co lambda for looking up sigmoid polynomial coefficients
            auto co = [&](int dx, int dy, int dz) {
                return cs->table[0uz
                + maxc      * math::pow(cs->table_res, 3) * 3
                + (zi + dz) * math::pow(cs->table_res, 2) * 3
                + (yi + dy) * math::pow(cs->table_res, 1) * 3
                + (xi + dx) * 3 + i
                ];
            };

            c[2 - i] = math::lerp(
                math::lerp(
                    math::lerp(co(0, 0, 0), co(1, 0, 0), dx),
                    math::lerp(co(0, 1, 0), co(1, 1, 0), dx),
                    dy
                ),
                math::lerp(
                    math::lerp(co(0, 0, 1), co(1, 0, 1), dx),
                    math::lerp(co(0, 1, 1), co(1, 1, 1), dx),
                    dy
                ),
                dz
            );
        }
    }

    auto Rgb_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        auto sigmoid = [](f32 x) -> f32 {
            if (std::isinf(x)) return x < 0.f ? 0.f : 1.f;
            return 0.5f + x / (2.f * math::sqrt(1.f + math::sqr(x)));
        };
        return s
        * sigmoid(math::polynomial(lambda, c))
        * (illuminant ? (*illuminant.data())(lambda) : 1.f);
    }
}
