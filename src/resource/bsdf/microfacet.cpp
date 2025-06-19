#include <metatron/resource/bsdf/microfacet.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <utility>

namespace metatron::bsdf {
	auto Microfacet_Bsdf::operator()(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi
	) const -> std::optional<Interaction> {
		auto conductive = spectra::max(exterior_k) > 0.f;
		auto reflective = -wo[1] * wi[1] > 0.f;
		if (conductive && !reflective) {
			return {};
		}
		auto eta = exterior_eta / interior_eta;
		auto wm = math::normalize(reflective ? wo + wi : wo * eta.value[0] + wi);

		return {};
	}

	auto Microfacet_Bsdf::sample(
		eval::Context const& ctx,
		math::Vector<f32, 3> const& u
	) const -> std::optional<Interaction> {
		return {};
	}

	auto Microfacet_Bsdf::clone(Attribute const& attr) const -> std::unique_ptr<Bsdf> {
		auto bsdf = std::make_unique<Microfacet_Bsdf>();
		bsdf->exterior_eta = attr.exterior_eta;
		bsdf->interior_eta = attr.interior_eta;
		bsdf->exterior_k = attr.exterior_k;
		bsdf->interior_k = attr.interior_k;
		bsdf->u_roughness = attr.u_roughness;
		bsdf->v_roughness = attr.v_roughness;

		if (attr.inside) {
			std::swap(bsdf->exterior_eta, bsdf->interior_eta);
			std::swap(bsdf->exterior_k, bsdf->interior_k);
		}
		return bsdf;
	}

	auto Microfacet_Bsdf::flags() const -> Flags {
		auto flags = 0;
		return Flags(flags);
	}
}
