#pragma once
#include <metatron/resource/eval/context.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/ray.hpp>
#include <unordered_set>
#include <typeindex>

namespace metatron::light {
	struct Interaction final {
		spectra::Stochastic_Spectrum L;
		math::Vector<f32, 3> wi;
		math::Vector<f32, 3> p;
		f32 t;
		f32 pdf;
	};

	struct Light {
		enum Flags {
			delta = 1 << 0,
		};

		auto virtual operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction> = 0;
		auto virtual sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction> = 0;
		auto virtual flags() const -> Flags = 0;

		auto static initialize() -> void;

	private:
		static std::unordered_set<std::type_index> delta_lights;
	};
}
