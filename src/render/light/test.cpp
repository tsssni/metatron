#include <metatron/render/light/test.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::light {
		Test_Emitter::Test_Emitter(std::vector<Light const*>&& lights, std::vector<Light const*>&& infinite_lights)
			: lights(std::move(lights)), infinite_lights(std::move(infinite_lights)) {}

		auto Test_Emitter::emit(math::Ray const& r) const -> std::unique_ptr<spectra::Spectrum> {
			return infinite_lights[0]->emit(r);
		}

		auto Test_Emitter::sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> Interaction {
			// TODO: not implemented
			return {};
		}
}
