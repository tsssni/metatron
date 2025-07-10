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
	::support<pro::skills::as_view>
	::support_copy<pro::constraint_level::nontrivial>
	::build {};
}
