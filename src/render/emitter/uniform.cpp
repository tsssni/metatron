#include <metatron/render/emitter/uniform.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::emitter {
		Uniform_Emitter::Uniform_Emitter(std::vector<Divider>&& dividers, std::vector<Divider>&& infinite_dividers)
			: dividers(std::move(dividers)), infinite_dividers(std::move(infinite_dividers)) {}

		auto Uniform_Emitter::operator()(light::Light const& light) const -> std::optional<emitter::Interaction> {
			if (&light != infinite_dividers.front().light) return {};
			return emitter::Interaction{
				&infinite_dividers.front(),
				1.f
			};
		}

		auto Uniform_Emitter::sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<emitter::Interaction> {
			return emitter::Interaction{
				&infinite_dividers.front(),
				1.f
			};
		}

		auto Uniform_Emitter::sample_infinite(
			eval::Context const& ctx,
			f32 u
		) const -> std::optional<emitter::Interaction> {
			return emitter::Interaction{
				&infinite_dividers.front(),
				1.f
			};
		}
}
