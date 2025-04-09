#include <metatron/volume/media/homogeneous.hpp>
#include <metatron/volume/phase/henyey-greenstein.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/spectra/constant.hpp>

namespace metatron::media {
	Homogeneous_Medium::Homogeneous_Medium(
		std::unique_ptr<spectra::Spectrum> sigma_a,
		std::unique_ptr<spectra::Spectrum> sigma_s,
		std::unique_ptr<spectra::Spectrum> Le
	): sigma_a(std::move(sigma_a)), sigma_s(std::move(sigma_s)), Le(std::move(Le)) {}

	auto Homogeneous_Medium::sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction> {
		auto lambda = ctx.L.lambda.front();
		auto sigma_t = (*sigma_a)(lambda) + (*sigma_s)(lambda);
		auto distr = math::Exponential_Distribution{sigma_t};

		auto t_u = distr.sample(u);
		auto t = std::min(t_u, t_max);
		auto pdf = t < t_max ? distr.pdf(t) : distr.pdf(t) / sigma_t;

		auto transmittance = ctx.L;
		auto sigma_a = ctx.L & (*this->sigma_a);
		auto sigma_s = ctx.L & (*this->sigma_s);
		auto sigma_n = ctx.L & spectra::Constant_Spectrum{0.f};
		auto sigma_maj = sigma_a + sigma_s + sigma_n;
		auto Le = ctx.L & (*this->Le);

		for (auto i = 0uz; i < transmittance.lambda.size(); i++) {
			transmittance.value[i] = std::exp(-sigma_maj.value[i] * t);
		}

		return Interaction{
			ctx.r.o + ctx.r.d * t,
			std::make_unique<phase::Henyey_Greenstein_Phase_Function>(0.5f),
			t,
			pdf,
			t < t_max ? sigma_maj * transmittance : transmittance,
			transmittance,
			sigma_a,
			sigma_s,
			sigma_n,
			sigma_maj,
			Le,
		};
	}
}
