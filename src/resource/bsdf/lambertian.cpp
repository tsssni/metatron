#include <metatron/resource/bsdf/lambertian.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/cosine-hemisphere.hpp>

namespace metatron::bsdf {
	auto Lambertian_Bsdf::operator()(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi
	) const -> std::optional<Interaction> {
		auto f = spectrum;
		f.value = math::foreach([&](f32 lambda, usize i) {
			if (-wo[1] * wi[1] >= 0.f) {
				return math::guarded_div(reflectance(lambda), math::pi);
			} else {
				return 0.f;
			}
		}, f.lambda);

		auto distr = math::Cosine_Hemisphere_Distribution{};
		auto pdf = distr.pdf(std::abs(wi[1]));
		pdf *= -wo[1] * wi[1] >= 0.f ? 1.f : 0.f;

		return Interaction{f, wi, pdf};
	}

	auto Lambertian_Bsdf::sample(
		eval::Context const& ctx,
		math::Vector<f32, 3> const& u
	) const -> std::optional<Interaction> {
		auto distr = math::Cosine_Hemisphere_Distribution{};
		auto wi = distr.sample({u[1], u[2]});
		auto pdf = distr.pdf(wi[1]);
		if (-ctx.r.d[1] * wi[1] < 0.f) {
			wi *= -1.f;
		}

		auto f = spectrum;
		f.value = math::foreach([&](f32 lambda, usize i) {
			return math::guarded_div(reflectance(lambda), math::pi);
		}, f.lambda);

		return Interaction{f, wi, pdf};
	}

	auto Lambertian_Bsdf::clone(Attribute const& attr) const -> std::unique_ptr<Bsdf> {
		auto bsdf = std::make_unique<Lambertian_Bsdf>();
		bsdf->spectrum = attr.spectrum;
		bsdf->reflectance = attr.reflectance;
		return bsdf;
	}

	auto Lambertian_Bsdf::flags() const -> Flags {
		return Flags::reflective;
	}
}
