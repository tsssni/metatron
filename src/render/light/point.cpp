#include <metatron/render/light/point.hpp>

namespace metatron::light {
	Point_Light::Point_Light(std::unique_ptr<spectra::Spectrum> L): L(std::move(L)) {}

	auto Point_Light::operator()(
		eval::Context const& ctx
	) const -> std::optional<Interaction> {
		return {};
	}

	auto Point_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const -> std::optional<Interaction> {
		auto wi = math::normalize(-ctx.r.o);
		auto r = math::length(ctx.r.o);
		return Interaction{
			(ctx.L & *L) / (r * r),
			wi,
			{0.f},
			r,
			1.f
		};
	}
}
