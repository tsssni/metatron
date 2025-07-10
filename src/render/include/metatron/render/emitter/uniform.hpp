#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/core/math/distribution/discrete.hpp>

namespace mtt::emitter {
	struct Uniform_Emitter final {
		Uniform_Emitter(
			std::vector<Divider>&& dividers,
			std::vector<Divider>&& infinite_dividers
		);
		auto operator()(
			eval::Context const& ctx,
			Divider const& divider
		) const noexcept -> std::optional<emitter::Interaction>;
		auto sample(
			eval::Context const& ctx,
			f32 u
		) const noexcept -> std::optional<emitter::Interaction>;
		auto sample_infinite(
			eval::Context const& ctx,
			f32 u
		) const noexcept -> std::optional<emitter::Interaction>;

	private:
		std::vector<Divider> dividers;
		std::vector<Divider> inf_dividers;
		math::Discrete_Distribution distr;
		math::Discrete_Distribution inf_distr;
	};
}
