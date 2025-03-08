#include <metatron/render/light/emitter.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::light {
		Emitter::Emitter(std::vector<Light const*>&& lights, std::vector<Light const*>&& infinite_lights)
			: lights(std::move(lights)), infinite_lights(std::move(infinite_lights)) {}

		auto Emitter::emit(math::Ray const& r) const -> std::unique_ptr<spectra::Spectrum> {
			return infinite_lights[0]->emit(r);
		}

		auto Emitter::sample(intr::Interaction const& intr, math::Vector<f32, 2> const& u) const -> Sample {
			// TODO: not implemented
			return {};
		}
}
