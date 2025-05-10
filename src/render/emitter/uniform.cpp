#include <metatron/render/emitter/uniform.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::emitter {
		Uniform_Emitter::Uniform_Emitter(
			std::vector<Divider>&& dividers,
			std::vector<Divider>&& infinite_dividers
		):
		dividers(std::move(dividers)),
		inf_dividers(std::move(infinite_dividers)),
		distr(std::vector<f32>(this->dividers.size() + this->inf_dividers.size(), 1.f / (this->dividers.size() + this->inf_dividers.size()))),
		inf_distr(std::vector<f32>(this->inf_dividers.size(), 1.f / this->inf_dividers.size())) {}

		auto Uniform_Emitter::operator()(
			eval::Context const& ctx,
			Divider const& divider
		) const -> std::optional<emitter::Interaction> {
			return emitter::Interaction{
				&divider,
				1.f / (dividers.size() + inf_dividers.size())
			};
		}

		auto Uniform_Emitter::sample(
			eval::Context const& ctx,
			f32 u
		) const -> std::optional<emitter::Interaction> {
			auto idx = distr.sample(u);
			return emitter::Interaction{
				idx < dividers.size()
				? &dividers[idx]
				: &inf_dividers[idx - dividers.size()],
				1.f / (dividers.size() + inf_dividers.size())
			};
		}

		auto Uniform_Emitter::sample_infinite(
			eval::Context const& ctx,
			f32 u
		) const -> std::optional<emitter::Interaction> {
			auto idx = inf_distr.sample(u);
			return emitter::Interaction{
				&inf_dividers[idx],
				1.f
			};
		}
}
