#include <metatron/render/light/uniform.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::light {
		Uniform_Emitter::Uniform_Emitter(std::vector<Light const*>&& lights, std::vector<Light const*>&& infinite_lights)
			: lights(std::move(lights)), infinite_lights(std::move(infinite_lights)) {}

		auto Uniform_Emitter::operator()(Light const& light) const -> std::optional<emitter::Interaction> {
			if (&light != infinite_lights.front()) return {};
			return emitter::Interaction{
				&light,
				1.f
			};
		}

		auto Uniform_Emitter::sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<emitter::Interaction> {
			return emitter::Interaction{
				infinite_lights.front(),
				1.f
			};
		}

		auto Uniform_Emitter::sample_infinite(eval::Context const& ctx, f32 u) const -> std::optional<emitter::Interaction> {
			return emitter::Interaction{
				infinite_lights.front(),
				1.f
			};
		}
}
