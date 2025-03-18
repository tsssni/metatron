#pragma once
#include <metatron/render/light/emitter.hpp>
#include <vector>

namespace metatron::light {
	struct Uniform_Emitter final: Emitter {
		Uniform_Emitter(std::vector<Light const*>&& lights, std::vector<Light const*>&& infinite_lights);
		auto operator()(Light const& light) const -> std::optional<emitter::Interaction>;
		auto sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<emitter::Interaction>;
		auto sample_infinite(eval::Context const& ctx, f32 u) const -> std::optional<emitter::Interaction>;

	private:
		std::vector<Light const*> lights;
		std::vector<Light const*> infinite_lights;
	};
}
