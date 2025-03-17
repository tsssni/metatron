#include <metatron/volume/media/homogeneous.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/spectra/constant.hpp>

namespace metatron::media {
	Homogeneous_Medium::Homogeneous_Medium(
		std::unique_ptr<phase::Phase_Function> phase,
		std::unique_ptr<spectra::Spectrum> sigma_a,
		std::unique_ptr<spectra::Spectrum> sigma_s,
		std::unique_ptr<spectra::Spectrum> Le
	): phase(std::move(phase)), sigma_a(std::move(sigma_a)), sigma_s(std::move(sigma_s)), Le(std::move(Le)) {}

	auto Homogeneous_Medium::sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction> {
		auto lambda = ctx.L->lambda.front();
		auto sigma_t = (*sigma_a)(lambda) + (*sigma_s)(lambda);
		auto distr = math::Exponential_Distribution{sigma_t};

		auto t = distr.sample(u);
		auto t_s = std::min(t, t_max);
		auto pdf = t < t_max ? distr(t_s) : distr(t_s) / sigma_t;

		auto transmittance = *ctx.L;
		auto sigma_a = transmittance & (*this->sigma_a);
		auto sigma_s = transmittance & (*this->sigma_s);
		auto sigma_n = transmittance & spectra::Constant_Spectrum{0.f};
		auto Le = transmittance & (*this->Le);

		for (auto i = 0uz; i < transmittance.lambda.size(); i++) {
			auto lambda = transmittance.lambda[i];
			auto sigma_t = sigma_a.value[i] + sigma_s.value[i];
			transmittance.value[i] = std::exp(-sigma_t * t_s);
		}

		return Interaction{
			ctx.r->o + ctx.r->d * t_s,
			phase.get(),
			t_s,
			pdf,
			std::make_unique<spectra::Stochastic_Spectrum>(transmittance),
			std::make_unique<spectra::Stochastic_Spectrum>(sigma_a),
			std::make_unique<spectra::Stochastic_Spectrum>(sigma_s),
			std::make_unique<spectra::Stochastic_Spectrum>(sigma_n),
			std::make_unique<spectra::Stochastic_Spectrum>(Le),
		};
	}
}
