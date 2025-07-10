#include <metatron/render/light/point.hpp>

namespace mtt::light {
	Point_Light::Point_Light(view<spectra::Spectrum> L) noexcept: L(L) {}

	auto Point_Light::operator()(
		eval::Context const& ctx
	) const noexcept -> std::optional<Interaction> {
		return {};
	}

	auto Point_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const noexcept -> std::optional<Interaction> {
		auto wi = math::normalize(-ctx.r.o);
		auto r = math::length(ctx.r.o);
		return Interaction{
			(ctx.spec & L) / (r * r),
			wi,
			{0.f},
			r,
			1.f
		};
	}

	auto Point_Light::flags() const noexcept -> Flags {
		return delta;
	}
}
