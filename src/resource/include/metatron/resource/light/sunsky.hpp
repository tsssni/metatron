#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/distribution/gaussian.hpp>
#include <metatron/core/math/distribution/cone.hpp>
#include <metatron/core/stl/arena.hpp>

namespace mtt::light {
    auto constexpr sunsky_num_lambda = 11;
    auto constexpr sunsky_num_turbility = 10;
    auto constexpr sunsky_num_albedo = 2;
    auto constexpr sunsky_lambda = std::array<f32, sunsky_num_lambda>{
        320.f, 360.f, 400.f, 440.f, 480.f, 520.f, 560.f, 600.f, 640.f, 680.f, 720.f
    };
    auto constexpr sunsky_step = 40.f;
    auto constexpr sky_num_params = 9;
    auto constexpr sky_num_ctls = 6;
    auto constexpr sun_num_ctls = 4;
    auto constexpr sun_num_segments = 45;
    auto constexpr sun_num_limb_params = 6;
    auto constexpr sun_aperture = 0.009351474132185619f;
    auto constexpr sun_blackbody_scale = math::pi * 3.19992e-10f;
    auto constexpr sun_perp_radiance = std::array<f32, sunsky_num_lambda>{
        7500.0f, 12500.0f, 21127.5f, 26760.5f, 30663.7f, 27825.0f,
        25503.8f, 25134.2f, 23212.1f, 21526.7f, 19870.8f,
    };
    auto constexpr tgmm_num_turbility = sunsky_num_turbility - 1;
    auto constexpr tgmm_num_segments = 30;
    auto constexpr tgmm_num_mixture = 5;
    auto constexpr tgmm_num_gaussian_params = 4;
    auto constexpr tgmm_num_bilinear = 4;
    auto constexpr tgmm_num_gaussian = tgmm_num_bilinear * tgmm_num_mixture;

    struct Sunsky_Light final {
        struct Descriptor final {
            fv2 direction;
            f32 turbidity = 1.0f;
            f32 albedo = 0.2f;
            f32 aperture = sun_aperture;
            f32 temperature = 6504.f;
            f32 intensity = 1.f;
        };
        Sunsky_Light() noexcept = default;
        Sunsky_Light(cref<Descriptor> desc) noexcept;

        auto static init() noexcept -> void;

        // Hosek atomosphere model: https://cgg.mff.cuni.cz/projects/SkylightModelling/
        // binary data: https://github.com/mitsuba-renderer/mitsuba-data/tree/master/sunsky/output
        auto operator()(
            cref<math::Ray> r, cref<fv4> lambda
        ) const noexcept -> opt<Interaction>;
        // TGMM sky sampling: https://diglib.eg.org/items/b3f1efca-1d13-44d0-ad60-741c4abe3d21
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;

    private:
        auto hosek(f32 lambda, f32 cos_theta, f32 cos_gamma) const noexcept -> f32;
        auto hosek_sky(i32 idx, f32 cos_theta, f32 cos_gamma) const noexcept -> f32;
        auto hosek_sun(i32 idx, f32 cos_theta) const noexcept -> f32;
        auto hosek_limb(i32 idx, f32 cos_gamma) const noexcept -> f32;
        auto hosek_integral() const noexcept -> f32;
        auto split(f32 lambda) const noexcept -> std::tuple<i32, i32, f32>;

        buf<f32> static sky_params_table;
        buf<f32> static sky_radiance_table;
        buf<f32> static sun_radiance_table;
        buf<f32> static sun_limb_table;
        buf<f32> static tgmm_table;

        fv3 d;
        fm44 t;

        f32 turbidity;
        f32 albedo;
        f32 cos_sun;
        f32 phi_sun;
        f32 area;
        f32 w_sky;

        fm<sunsky_num_lambda, sky_num_params> sky_params;
        fv<sunsky_num_lambda> sky_radiance;
        fm<sun_num_segments, sunsky_num_lambda, sun_num_ctls> sun_radiance;
        std::array<math::Truncated_Gaussian_Distribution, tgmm_num_gaussian> tgmm_phi_distr;
        std::array<math::Truncated_Gaussian_Distribution, tgmm_num_gaussian> tgmm_theta_distr;
        math::Discrete_Distribution<tgmm_num_gaussian> tgmm_distr;
        math::Cone_Distribution sun_distr;
    };
}
