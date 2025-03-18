#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <vector>

namespace metatron::emitter {
	struct Uniform_Emitter final: Emitter {
		Uniform_Emitter(std::vector<Divider>&& dividers, std::vector<Divider>&& infinite_dividers);
		auto operator()(light::Light const& light) const -> std::optional<emitter::Interaction>;
		auto sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<emitter::Interaction>;
		auto sample_infinite(eval::Context const& ctx, f32 u) const -> std::optional<emitter::Interaction>;

	private:
		std::vector<Divider> dividers;
		std::vector<Divider> infinite_dividers;
	};
}
