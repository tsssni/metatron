#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/vector.hpp>
#include <unordered_map>

namespace mtt::bsdf {
	struct Attribute final {
		std::unordered_map<std::string, spectra::Stochastic_Spectrum> spectra;
		std::unordered_map<std::string, math::Vector<f32, 4>> vectors;
		bool inside;
	};

	struct Interaction final {
		spectra::Stochastic_Spectrum f;
		math::Vector<f32, 3> wi;
		f32 pdf;
		bool degraded{false};
	};

	MTT_POLY_METHOD(bsdf_sample, sample);
	MTT_POLY_METHOD(bsdf_configure, configure);
	MTT_POLY_METHOD(bsdf_flags, flags);
	MTT_POLY_METHOD(bsdf_degrade, degrade);

	enum Flags {
		reflective = 1 << 0,
		transmissive = 1 << 1,
		interface = 1 << 2,
	};

	struct Bsdf final: pro::facade_builder
	::add_convention<pro::operator_dispatch<"()">, auto (
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi
	) const noexcept -> std::optional<Interaction>>
	::add_convention<bsdf_sample, auto (
		eval::Context const& ctx,
		math::Vector<f32, 3> const& u
	) const noexcept -> std::optional<Interaction>>
	::add_convention<bsdf_configure, auto (
		Attribute const& attr
	) noexcept -> void>
	::add_convention<bsdf_flags, auto () const noexcept -> Flags>
	::add_convention<bsdf_degrade, auto () noexcept -> bool>
	::add_skill<pro::skills::as_view>
	::build {};

	auto lambert(f32 reflectance) -> f32;
	auto lambert(spectra::Stochastic_Spectrum const& reflectance) -> spectra::Stochastic_Spectrum;

	auto fresnel(f32 cos_theta_i, f32 eta, f32 k) -> f32;
	auto fresnel(
		f32 cos_theta_i,
		spectra::Stochastic_Spectrum const& eta,
		spectra::Stochastic_Spectrum const& k
	) noexcept -> spectra::Stochastic_Spectrum;

	auto lambda(
		math::Vector<f32, 3> const& wo,
		f32 alpha_u,
		f32 alpha_v
	) noexcept -> f32;
	auto smith_mask(
		math::Vector<f32, 3> const& wo,
		f32 alpha_u,
		f32 alpha_v
	) noexcept -> f32;
	auto smith_shadow(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi,
		f32 alpha_u,
		f32 alpha_v
	) noexcept -> f32;

	auto trowbridge_reitz(
		math::Vector<f32, 3> const& wm,
		f32 alpha_u,
		f32 alpha_v
	) noexcept -> f32;
	auto visible_trowbridge_reitz(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wm,
		f32 alpha_u,
		f32 alpha_v
	) noexcept -> f32;
	auto torrance_sparrow(
		bool reflective, f32 pr, f32 pt,
		spectra::Stochastic_Spectrum const& F, f32 D, f32 G,
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi,
		math::Vector<f32, 3> const& wm,
		spectra::Stochastic_Spectrum const& eta,
		f32 alpha_u,
		f32 alpha_v
	) noexcept -> std::optional<Interaction>;
}
