#pragma once
#include <metatron/render/light/light.hpp>
#include <vector>

namespace metatron::light {
	struct Emitter final: Light {
		Emitter(std::vector<Light const*>&& lights, std::vector<Light const*>&& infinite_lights);
		auto emit(math::Ray const& r) const -> std::unique_ptr<spectra::Spectrum>;
		auto sample(intr::Interaction const& intr, math::Vector<f32, 2> const& u) const -> Sample;

	private:
		std::vector<Light const*> lights;
		std::vector<Light const*> infinite_lights;
	};
}
