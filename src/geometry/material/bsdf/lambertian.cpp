#include <metatron/geometry/material/bsdf/lambertian.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/cosine-hemisphere.hpp>

namespace metatron::material {
	Lambertian_Bsdf::Lambertian_Bsdf(
		std::unique_ptr<spectra::Stochastic_Spectrum> R,
		std::unique_ptr<spectra::Stochastic_Spectrum> T
	) : R(std::move(R)), T(std::move(T)) {
		*(this->R) /= math::pi;
		*(this->T) /= math::pi;
	}

	auto Lambertian_Bsdf::operator()(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi,
		spectra::Stochastic_Spectrum const& L
	) const -> std::optional<bsdf::Interaction> {
		auto ru = spectra::max(*R);
		auto tu = spectra::max(*T);
		if (std::abs(ru + tu) < math::epsilon<f32>) return {};

		auto f = L;
		for (auto i = 0uz; i < f.lambda.size(); i++) {
			if (-wo[1] * wi[1] >= 0.f) f.value[i] = (*R)(f.lambda[i]);
			else f.value[i] = (*T)(f.lambda[i]);
		}

		auto pdf = math::Cosine_Hemisphere_Distribution::pdf(std::abs(wi[1]));
		pdf *= (-wo[1] * wi[1] < 0.f ? ru : tu) / (ru + tu);

		return bsdf::Interaction{f, wi, pdf};
	}

	auto Lambertian_Bsdf::sample(eval::Context const& ctx, math::Vector<f32, 3> const& u) const -> std::optional<bsdf::Interaction> {
		auto ru = spectra::max(*R);
		auto tu = spectra::max(*T);
		if (std::abs(ru + tu) < math::epsilon<f32>) return {};

		auto rtu = ru / (ru + tu);
		auto wi = math::Cosine_Hemisphere_Distribution::sample({u[1], u[2]});
		auto pdf = math::Cosine_Hemisphere_Distribution::pdf(wi[1]);

		if (u[0] < rtu) {
			pdf *= rtu;
			if (-ctx.r.d[1] * wi[1] < 0.f) {
				wi *= -1.f;
			}
		} else {
			pdf *= 1.f - rtu;
			if (-ctx.r.d[1] * wi[1] >= 0.f) {
				wi *= -1.f;
			}
		}

		auto f = ctx.L;
		for (auto i = 0uz; i < f.lambda.size(); i++) {
			if (-ctx.r.d[1] * wi[1] >= 0.f) f.value[i] = (*R)(f.lambda[i]);
			else f.value[i] = (*T)(f.lambda[i]);
		}
		return bsdf::Interaction{f, wi, pdf};
	}
}
