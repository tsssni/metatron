#include <metatron/render/light/parallel.hpp>

namespace mtt::light {
	Parallel_Light::Parallel_Light(pro::proxy_view<spectra::Spectrum> L): L(L) {}

	auto Parallel_Light::operator()(
		eval::Context const& ctx
	) const -> std::optional<Interaction> {
		return {};
	}

	auto Parallel_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const -> std::optional<Interaction> {
		auto constexpr wi = math::Vector<f32, 3>{0.f, 0.f, -1.f};
		return Interaction{
			ctx.spec & L,
			wi,
			ctx.r.o - 65535.f * wi,
			65535.f,
			1.f
		};
	}

	auto Parallel_Light::flags() const -> Flags {
		return delta;
	}
}
