#include <metatron/geometry/material/bsdf/cross.hpp>

namespace metatron::material {
	Cross_Bsdf::Cross_Bsdf() = default;

	auto Cross_Bsdf::operator()(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi,
		spectra::Stochastic_Spectrum const& L
	) const -> std::optional<bsdf::Interaction> {
		auto crossed = f32(wo == wi);
		auto f = L;
		f.value = std::vector<f32>(f.lambda.size(), crossed);
		return bsdf::Interaction{f, wi, crossed};
	}

	auto Cross_Bsdf::sample(eval::Context const& ctx, math::Vector<f32, 3> const& u) const -> std::optional<bsdf::Interaction> {
		auto f = ctx.L;
		f.value = std::vector<f32>(f.lambda.size(), 1.f);
		return bsdf::Interaction{f, ctx.r.d, 1.f};
	}
}
