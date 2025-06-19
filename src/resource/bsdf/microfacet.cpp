#include <metatron/resource/bsdf/microfacet.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <utility>

namespace metatron::bsdf {
	auto Microfacet_Bsdf::operator()(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi
	) const -> std::optional<Interaction> {
		if (false
		|| math::abs(wo[1]) < math::epsilon<f32>
		|| math::abs(wi[1]) < math::epsilon<f32>
		) {
			return {};
		}

		auto conductive = spectra::max(interior_k) > math::epsilon<f32>;
		auto reflective = -wo[1] * wi[1] > 0.f;
		if (conductive && !reflective) {
			return {};
		}

		auto eta = 1.f
		* math::Complex<f32>{interior_eta.value[0], interior_k.value[0]}
		/ math::Complex<f32>{exterior_eta.value[0], exterior_k.value[0]};
		auto wm = math::normalize(reflective ? wo + wi : wo * eta.r + wi);
		if (math::all(
			[](f32 v, usize) { return v < math::epsilon<f32>; },
			math::abs(wm)
		)) {
			return {};
		}

		auto F = fresnel(math::dot(-wo, wm), eta);
		auto D = trowbridge_reitz(wm);
		auto G = smith(wo, wi);

		auto pr = F;
		auto pt = 1.f - F;
		auto flags = this->flags();
		if (!(flags & bsdf::Bsdf::reflective)) {
			pr = 0.f;
		}
		if (!(flags & bsdf::Bsdf::transmissive)) {
			pt = 0.f;
		}
		if (pr == 0.f && pt == 0.f) {
			return {};
		}

		auto cos_theta_o = math::unit_to_cos_theta(-wo);
		auto cos_theta_i = math::unit_to_cos_theta(wi);
		auto cos_theta_om = math::dot(-wo, wm);
		auto cos_theta_im = math::dot(wi, wm);

		auto f = exterior_eta;
		auto pdf = 1.f;
		if (reflective) {
			f = F * D * G / math::abs(4.f * cos_theta_o * cos_theta_i);
			pdf = D / (4.f * math::abs(cos_theta_om)) * pr / (pr + pt);
		} else {
			auto denom = math::sqr(cos_theta_im + cos_theta_om / eta.r);
			f = (1.f - F) * D * G
			* math::abs(cos_theta_om * cos_theta_im / (denom * cos_theta_i * cos_theta_o))
			/ math::sqr(eta.r);
			pdf = D * math::abs(cos_theta_im) / denom * pt / (pr + pt); 
		}

		return Interaction{f, wi, pdf};
	}

	auto Microfacet_Bsdf::sample(
		eval::Context const& ctx,
		math::Vector<f32, 3> const& u
	) const -> std::optional<Interaction> {
		auto wo = ctx.r.d;
		return {};
	}

	auto Microfacet_Bsdf::clone(Attribute const& attr) const -> std::unique_ptr<Bsdf> {
		auto bsdf = std::make_unique<Microfacet_Bsdf>();
		bsdf->exterior_eta = attr.exterior_eta;
		bsdf->interior_eta = attr.interior_eta;
		bsdf->exterior_k = attr.exterior_k;
		bsdf->interior_k = attr.interior_k;
		bsdf->alpha_x = attr.u_roughness;
		bsdf->alpha_y = attr.v_roughness;

		if (attr.inside) {
			std::swap(bsdf->exterior_eta, bsdf->interior_eta);
			std::swap(bsdf->exterior_k, bsdf->interior_k);
		}
		return bsdf;
	}

	auto Microfacet_Bsdf::flags() const -> Flags {
		auto flags = 0;
		auto conductive = spectra::max(interior_k) > math::epsilon<f32>;
		if (conductive) {
			flags |= bsdf::Bsdf::reflective;
		} else if (interior_eta.value[0] == interior_eta.value[1]) {
			flags |= bsdf::Bsdf::transmissive;
		} else {
			flags |= (bsdf::Bsdf::transmissive | bsdf::Bsdf::reflective);
		}
		return Flags(flags);
	}


	auto Microfacet_Bsdf::fresnel(f32 cos_theta_i, math::Complex<f32> eta) const -> f32 {
		cos_theta_i = std::clamp(cos_theta_i, -1.f, 1.f);
		auto sin2_theta_i = std::max(0.f, 1.f - cos_theta_i * cos_theta_i);
		auto sin2_theta_t = sin2_theta_i / (eta * eta);
		auto cos_theta_t = math::sqrt(1.f - sin2_theta_t);

		auto conductive = eta.i > math::epsilon<f32>;
		if (!conductive && sin2_theta_t.r >= 1.f) {
			return 1.f;
		}

		auto r_parl = 1.f
		* (eta * cos_theta_i - cos_theta_t)
		/ (eta * cos_theta_i + cos_theta_t);
		auto r_perp = 1.f
		* (cos_theta_i - eta * cos_theta_t)
		/ (cos_theta_i + eta * cos_theta_t);
		return (math::norm(r_parl) + math::norm(r_perp)) / 2.f;
	}

	auto Microfacet_Bsdf::trowbridge_reitz(math::Vector<f32, 3> const& wm) const -> f32 {
		auto tan2_theta = math::unit_to_tan2_theta(wm);
		if (std::isinf(tan2_theta)) {
			return 0.f;
		}

		auto cos2_theta = math::unit_to_cos2_theta(wm);
		auto cos4_theta = math::sqr(cos2_theta);
		if (cos4_theta < math::epsilon<f32>) {
			return 0.f;
		}

		auto cos_phi = math::unit_to_cos_phi(wm);
		auto sin_phi = math::unit_to_sin_phi(wm);
		auto e = tan2_theta * (0.f
		+ math::sqr(cos_phi / alpha_x)
		+ math::sqr(sin_phi / alpha_y));

		return 1.f / (math::pi * alpha_x * alpha_y * cos4_theta * math::sqr(1.f + e));
	}

	auto Microfacet_Bsdf::smith(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wi) const -> f32 {
		auto lambda = [&](math::Vector<f32, 3> const& w) {
			auto tan2_theta = math::unit_to_tan2_theta(w);
			if (std::isinf(tan2_theta)) {
				return 0.f;
			}
			auto alpha2 = 0.f
			+ math::sqr(math::unit_to_cos_theta(w) * alpha_x)
			+ math::sqr(math::unit_to_sin_theta(w) * alpha_y);
			return (math::sqrt(1.f + alpha2 * tan2_theta) - 1.f) / 2.f;
		};
		return 1.f / (1.f + lambda(-wo) + lambda(wi));
	}
}
