#include <metatron/render/light/parallel.hpp>
#include <metatron/core/spectra/constant.hpp>

namespace metatron::light {
	Parallel_Light::Parallel_Light(
		std::unique_ptr<spectra::Spectrum> L,
		math::Vector<f32, 3> const& d
	): L(std::move(L)), d(d) {}

	auto Parallel_Light::operator()(
		eval::Context const& ctx
	) const -> std::optional<Interaction> {
		return Interaction{
			math::length(ctx.r.d - d) < math::epsilon<f32>
			? ctx.L & *L
			: ctx.L & spectra::Constant_Spectrum{0.f},
			{}, {}, {}, 1.f
		};
	}

	auto Parallel_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const -> std::optional<Interaction> {
		return Interaction{
			ctx.L & *L,
			-d,
			ctx.r.o + 65535.f * d,
			65535.f,
			1.f
		};
	}
}
