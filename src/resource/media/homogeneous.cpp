#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace mtt::media {
	Homogeneous_Medium::Homogeneous_Medium(
		phase::Phase_Function const* phase,
		view<spectra::Spectrum> sigma_a,
		view<spectra::Spectrum> sigma_s,
		view<spectra::Spectrum> emission
	):
	phase{phase},
	sigma_a{sigma_a},
	sigma_s{sigma_s},
	emission{emission} {}

	auto Homogeneous_Medium::sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction> {
		auto sigma_a = ctx.spec & this->sigma_a;
		auto sigma_s = ctx.spec & this->sigma_s;
		auto sigma_t = sigma_a + sigma_s;
		auto sigma_maj = sigma_t;
		auto sigma_n = ctx.spec & pro::make_proxy<spectra::Spectrum, spectra::Constant_Spectrum>(0.f);

		auto distr = math::Exponential_Distribution{sigma_t.value[0]};
		auto t_u = distr.sample(u);
		auto t = std::min(t_u, t_max);
		auto pdf = t < t_max ? distr.pdf(t) : distr.pdf(t) / sigma_t.value[0];

		auto emission = ctx.spec & this->emission;
		auto transmittance = ctx.spec;
		transmittance.value = math::foreach([&](f32 value, usize i) {
			return std::exp(-value * t);
		}, transmittance.value);

		return Interaction{
			ctx.r.o + ctx.r.d * t,
			phase->clone({ctx.spec}),
			t,
			pdf,
			t < t_max ? sigma_maj * transmittance : transmittance,
			transmittance,
			sigma_a,
			sigma_s,
			sigma_n,
			sigma_maj,
			emission
		};
	}
}
