#include <metatron/geometry/material/bsdf/lambertian.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/cosine-hemisphere.hpp>

namespace metatron::material {
	Lambertian_Bsdf::Lambertian_Bsdf(std::unique_ptr<spectra::Spectrum> R, std::unique_ptr<spectra::Spectrum> T)
		: R(std::move(R)), T(std::move(T)) {}

	auto Lambertian_Bsdf::operator()(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi,
		f32 lambda
	) const -> f32 {
		if (wo[1] * wi[1] >= 0.f) return (*R)(lambda);
		else return (*T)(lambda);
	}

	auto Lambertian_Bsdf::sample(bsdf::Context const& ctx, math::Vector<f32, 3> const& u) const -> std::optional<bsdf::Interaction> {
		auto r = ((*ctx.Lo) & (*R)) / math::pi;
		auto t = ((*ctx.Lo) & (*T)) / math::pi;
		auto ru = spectra::max(r);
		auto tu = spectra::max(t);
		if (std::abs(ru + tu) < math::epsilon<f32>) return {};

		auto rtu = ru / (ru + tu);
		auto wi = math::Cosine_Hemisphere_Distribution::sample({u[1], u[2]});
		auto pdf = math::Cosine_Hemisphere_Distribution::pdf(wi[1]);

		if (u[0] < rtu) {
			pdf *= rtu;
			if (-ctx.wo[1] * wi[1] < 0.f) {
				wi *= -1.f;
			}
		} else {
			pdf *= 1.f - rtu;
			if (-ctx.wo[1] * wi[1] >= 0.f) {
				wi *= -1.f;
			}
		}
		
		auto f = std::make_unique<spectra::Stochastic_Spectrum>(*ctx.Lo);
		for (auto i = 0uz; i < f->lambda.size(); i++) {
			f->value[i] = (*this)(-ctx.wo, wi, f->lambda[i]);
		}
		return bsdf::Interaction{std::move(f), wi, pdf};
	}
}
