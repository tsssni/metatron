#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/distribution/discrete.hpp>

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
	auto constexpr tgmm_num_turbility = sunsky_num_turbility - 1;
	auto constexpr tgmm_num_segments = 30;
	auto constexpr tgmm_num_mixture = 5;
	auto constexpr tgmm_num_gaussian_params = 4;
	auto constexpr tgmm_num_bilinear = 4;
	auto constexpr tgmm_num_gaussian = tgmm_num_bilinear * tgmm_num_mixture;

	struct Sunsky_Light final {
		Sunsky_Light(
			math::Vector<f32, 2> direction,
			f32 turbidity,
			f32 albedo,
			f32 aperture
		) noexcept;

		auto static init() noexcept -> void;

		// Hosek atomosphere model: https://cgg.mff.cuni.cz/projects/SkylightModelling/
		// binary data: https://github.com/mitsuba-renderer/mitsuba-data/tree/master/sunsky/output
		auto operator()(
			eval::Context const& ctx
		) const noexcept -> std::optional<Interaction>;
		// TGMM sky sampling: https://diglib.eg.org/items/b3f1efca-1d13-44d0-ad60-741c4abe3d21
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const noexcept -> std::optional<Interaction>;
		auto flags() const noexcept -> Flags;

	private:
		auto hosek(f32 lambda, f32 cos_theta, f32 cos_gamma) const noexcept -> f32;
		auto hosek_sky(f32 lambda, f32 cos_theta, f32 cos_gamma) const noexcept -> f32;
		auto hosek_sky(i32 idx, f32 cos_theta, f32 cos_gamma) const noexcept -> f32;
		auto hosek_sun(f32 lambda, f32 cos_theta, f32 cos_gamma) const noexcept -> f32;
		auto hosek_sun(i32 idx, f32 cos_theta) const noexcept -> f32;
		auto hosek_limb(i32 idx, f32 cos_gamma) const noexcept -> f32;
		auto hosek_integral() const noexcept -> f32;

		auto sample_sky(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const noexcept -> std::optional<Interaction>;
		auto sample_sun(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const noexcept -> std::optional<Interaction>;

		auto split(f32 lambda) const noexcept -> std::tuple<i32, i32, f32>;

		std::vector<f32> static sky_params_table;
		std::vector<f32> static sky_radiance_table;
		std::vector<f32> static sun_radiance_table;
		std::vector<f32> static sun_limb_table;
		std::vector<f32> static tgmm_table;

		math::Vector<f32, 3> d;
		math::Matrix<f32, 4, 4> t;

		f32 turbidity;
		f32 albedo;
		f32 cos_sun;
		f32 phi_sun;
		f32 area;
		f32 w_sky;

		math::Matrix<f32, sunsky_num_lambda, sky_num_params> sky_params;
		math::Vector<f32, sunsky_num_lambda> sky_radiance;
		math::Matrix<f32, sun_num_segments, sunsky_num_lambda, sun_num_ctls> sun_radiance;
		math::Matrix<f32, tgmm_num_gaussian, tgmm_num_gaussian_params> tgmm_gaussian;
		math::Discrete_Distribution tgmm_distr{{}};
	};
}
