#pragma once
#include <metatron/render/light/emitter.hpp>
#include <vector>

namespace metatron::light {
	struct Test_Emitter final: Emitter {
		Test_Emitter(std::vector<Light const*>&& lights, std::vector<Light const*>&& infinite_lights);
		auto operator()(math::Ray const& r) const -> std::optional<Light const*>;
		auto sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<emitter::Interaction>;

	private:
		std::vector<Light const*> lights;
		std::vector<Light const*> infinite_lights;
	};
}
