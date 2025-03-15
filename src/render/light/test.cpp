#include <metatron/render/light/test.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::light {
		Test_Emitter::Test_Emitter(std::vector<Light const*>&& lights, std::vector<Light const*>&& infinite_lights)
			: lights(std::move(lights)), infinite_lights(std::move(infinite_lights)) {}

		auto Test_Emitter::operator()(math::Ray const& r) const -> std::optional<Interaction> {
			return (*infinite_lights[0])(r);
		}

		auto Test_Emitter::sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> {
			// TODO: not implemented
			return {};
		}
}
