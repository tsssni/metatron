#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace metatron::media {
	Homogeneous_Medium::Homogeneous_Medium(
		std::unique_ptr<spectra::Spectrum> sigma_a,
		std::unique_ptr<spectra::Spectrum> sigma_s,
		std::unique_ptr<spectra::Spectrum> L,
		f32 g
	):
	sigma_a(std::move(sigma_a)),
	sigma_s(std::move(sigma_s)),
	L(std::move(L)),
	g(g) {}

	auto Homogeneous_Medium::sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction> {
		auto sigma_a = ctx.L & (*this->sigma_a);
		auto sigma_s = ctx.L & (*this->sigma_s);
		auto sigma_t = sigma_a + sigma_s;
		auto sigma_maj = sigma_t;
		auto sigma_n = ctx.L & spectra::Constant_Spectrum{0.f};

		auto distr = math::Exponential_Distribution{sigma_t.value.front()};
		auto t_u = distr.sample(u);
		auto t = std::min(t_u, t_max);
		auto pdf = t < t_max ? distr.pdf(t) : distr.pdf(t) / sigma_t.value.front();

		auto transmittance = ctx.L;
		auto L = ctx.L & (*this->L);

		for (auto i = 0uz; i < transmittance.lambda.size(); i++) {
			transmittance.value[i] = std::exp(-sigma_maj.value[i] * t);
		}

		return Interaction{
			ctx.r.o + ctx.r.d * t,
			std::make_unique<phase::Henyey_Greenstein_Phase_Function>(g),
			t,
			pdf,
			t < t_max ? sigma_maj * transmittance : transmittance,
			transmittance,
			sigma_a,
			sigma_s,
			sigma_n,
			sigma_maj,
			L,
		};
	}
}
