#include <metatron/render/light/parallel.hpp>
#include <metatron/core/spectra/constant.hpp>

namespace metatron::light {
	Parallel_Light::Parallel_Light(
		std::unique_ptr<spectra::Spectrum> L,
		math::Vector<f32, 3> const& sphere_coord
	): L(std::move(L)), sphere_coord(sphere_coord) {}

	auto Parallel_Light::operator()(
		eval::Context const& ctx
	) const -> std::optional<Interaction> {
		return {};
	}

	auto Parallel_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const -> std::optional<Interaction> {
		return Interaction{
			ctx.L & *L,
			sphere_coord,
			ctx.r.o + 65535.f * sphere_coord,
			65535.f,
			1.f
		};
	}
}
