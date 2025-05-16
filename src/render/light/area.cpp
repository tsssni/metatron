#include <metatron/render/light/area.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::light {
	Area_Light::Area_Light(
		shape::Shape const& shape,
		usize idx
	): shape(&shape), idx(idx) {}

	auto Area_Light::operator()(
		eval::Context const& ctx
	) const -> std::optional<Interaction> {
		return {};
	}

	auto Area_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const -> std::optional<Interaction> {
		OPTIONAL_OR_RETURN(s_intr, shape->sample(ctx, u), {});
		auto L = ctx.L; L = 0.f; // delay fetching L in integrator material interaction
		return Interaction{
			L,
			math::normalize(s_intr.p - ctx.r.o),
			s_intr.p,
			s_intr.t,
			s_intr.pdf
		};
	}
}
