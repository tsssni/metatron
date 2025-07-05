#include <metatron/resource/media/vaccum.hpp>

namespace mtt::media {
	Vaccum_Medium::Vaccum_Medium() {}

	auto Vaccum_Medium::sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction> {
		auto transmittance = ctx.spec;
		transmittance = 1.f;

		return Interaction{
			ctx.r.o + t_max * ctx.r.d,
			{},
			t_max,
			1.f,
			transmittance,
			transmittance,
			{}, {}, {}, {}, {}
		};
	}
}
