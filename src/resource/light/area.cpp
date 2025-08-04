#include <metatron/resource/light/area.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::light {
	Area_Light::Area_Light(
		view<shape::Shape> shape,
		usize primitive
	) noexcept: shape(shape), primitive(primitive) {}

	auto Area_Light::operator()(
		eval::Context const& ctx
	) const noexcept -> std::optional<Interaction> {
		return {};
	}

	auto Area_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const noexcept -> std::optional<Interaction> {
		MTT_OPT_OR_RETURN(s_intr, shape->sample(ctx, u, primitive), {});
		auto L = ctx.spec; L = 0.f; // delay fetching L in integrator material interaction
		return Interaction{
			L,
			math::normalize(s_intr.p - ctx.r.o),
			s_intr.p,
			s_intr.t,
			s_intr.pdf
		};
	}

	auto Area_Light::flags() const noexcept -> Flags {
		return Flags(0);
	}
}
