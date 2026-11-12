#pragma once
#include <metatron/render/light/interaction.hpp>
#include <metatron/core/distribution/cone.hpp>
#include <metatron/core/distribution/discrete.hpp>
#include <metatron/core/distribution/gaussian.hpp>

namespace mtt::render {

auto init_atmosphere() noexcept -> void;

auto constexpr atmo_num_lambda = 11;
auto constexpr atmo_num_turbility = 10;
auto constexpr atmo_num_albedo = 2;
auto constexpr atmo_lambda = std::array<f32, atmo_num_lambda>{
    320.f, 360.f, 400.f, 440.f, 480.f, 520.f, 560.f, 600.f, 640.f, 680.f, 720.f
};
auto constexpr atmo_step = 40.f;
auto constexpr sky_num_params = 9;
auto constexpr sky_num_ctls = 6;
auto constexpr sun_num_ctls = 4;
auto constexpr sun_num_segments = 45;
auto constexpr sun_num_limb_params = 6;
auto constexpr sun_aperture = 0.009351474132185619f;
auto constexpr sun_blackbody_scale = core::pi * 3.19992e-10f;
auto constexpr sun_perp_radiance = std::array<f32, atmo_num_lambda>{
    7500.0f, 12500.0f, 21127.5f, 26760.5f, 30663.7f, 27825.0f, 25503.8f, 25134.2f, 23212.1f, 21526.7f, 19870.8f
};
auto constexpr tgmm_num_turbility = atmo_num_turbility - 1;
auto constexpr tgmm_num_segments = 30;
auto constexpr tgmm_num_mixture = 5;
auto constexpr tgmm_num_gaussian_params = 4;
auto constexpr tgmm_num_bilinear = 4;
auto constexpr tgmm_num_gaussian = tgmm_num_bilinear * tgmm_num_mixture;

struct [[mtt::reflection(CIE_Y = "/spectra/CIE-Y")]] Atmosphere_Light final {
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
    buf<f32> sky_params;
    buf<f32> sky_radiance;
    buf<f32> sun_radiance;
    buf<f32> sun_limb;
    buf<core::Truncated_Gaussian_Distribution> tgmm_phi_distr;
    buf<core::Truncated_Gaussian_Distribution> tgmm_theta_distr;
    core::Array_Distribution tgmm_distr;
    core::Cone_Distribution sun_distr;

    Atmosphere_Light(
        fv2 direction,
        f32 turbidity,
        f32 albedo,
        f32 aperture,
        f32 temperature,
        f32 intensity,
        Spectrum CIE_Y,
        core::recorder& recorder
    ) noexcept;

    // Hosek atmosphere model: https://cgg.mff.cuni.cz/projects/SkylightModelling/
    // binary data: https://github.com/mitsuba-renderer/mitsuba-data/tree/master/sunsky/output
    auto operator()(
        core::Ray const& r, fv4 const& lambda
    ) const noexcept -> Light_Interaction;
    // TGMM sky sampling: https://diglib.eg.org/items/b3f1efca-1d13-44d0-ad60-741c4abe3d21
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> Light_Interaction;
    auto flags() const noexcept -> Light_Flags;

    struct State {
        f32 cos_theta;
        f32 cos_gamma;
        f32 gamma;
        f32 sqrt_cos_theta;
        i32 segment;
        f32 x;
        f32 cos_psi;
        State(f32 cos_theta, f32 cos_gamma, f32 cos_sun) noexcept;
    };

    auto hosek_sky(i32 idx, State const& s) const noexcept -> f32;
    auto hosek_sun(i32 idx, State const& s) const noexcept -> f32;
    auto hosek_limb(i32 idx, State const& s) const noexcept -> f32;

private:
    auto hosek(f32 lambda, State const& s) const noexcept -> f32;
    auto split(f32 lambda) const noexcept -> std::tuple<i32, i32, f32>;
};

}
