#include <metatron/resource/bsdf/microfacet.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/disk.hpp>
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

		auto wm = math::normalize(reflective ? -wo + wi : -wo + wi * eta.r);
		if (wm[1] < 0.f) {
			wm *= -1.f;
		}
		if (false
		|| math::abs(wm[1]) < math::epsilon<f32>
		|| math::dot(-wo, wm) < 0.f
		|| math::dot((reflective ? 1.f : -1.f) * wi, wm) < 0.f) {
			return {};
		}

		auto F = fresnel(math::dot(-wo, wm), eta);
		auto D = trowbridge_reitz(wm);
		auto G = smith_shadow(wo, wi);

		if (F > 1.f) {
			std::printf("\n%f %f %f %f %f %f %f %f %f\n",
				-wo[0], -wo[1], -wo[2],
				wi[0], wi[1], wi[2],
				wm[0], wm[1], wm[2]);
		}

		auto pr = F;
		auto pt = 1.f - F;
		auto flags = this->flags();
		pr *= (flags & bsdf::Bsdf::reflective);
		pt *= (flags & bsdf::Bsdf::transmissive);
		if (pr == 0.f && pt == 0.f) {
			return {};
		}

		return torrance_sparrow(reflective, pr, pt, F, D, G, wo, wi, wm);
	}

	auto Microfacet_Bsdf::sample(
		eval::Context const& ctx,
		math::Vector<f32, 3> const& u
	) const -> std::optional<Interaction> {
		auto wo = ctx.r.d;
		
		auto wy = math::normalize(-wo * math::Vector<f32, 3>{u_roughness, 1.f, v_roughness});
		auto wx = wy[1] < 1.f - math::epsilon<f32>
			? math::cross(wy, math::Vector<f32, 3>{0.f, 1.f, 0.f})
			: math::Vector<f32, 3>{1.f, 0.f, 0.f};
		auto wz = math::cross(wx, wy);

		// use polar disk distribution to fetch more samples near center
		auto distr = math::Polar_Disk_Distribution{};
		auto sample_p = distr.sample({u[1], u[2]});
		auto sample_h = math::sqrt(1.f - math::sqr(sample_p[0]));
		sample_p[1] = (1.f + wy[1]) / 2.f * sample_p[1] + (1.f - wy[1]) * sample_h / 2.f;

		auto sample_y = math::sqrt(1.f - math::dot(sample_p, sample_p));
		auto wm = sample_p[0] * wx + sample_y * wy + sample_p[1] * wz;
		if (wm[1] < math::epsilon<f32>) {
			return {};
		}
		// normal transformation with inverse transposed matrix
		wm = math::normalize(wm * math::Vector<f32, 3>{u_roughness, 1.f, v_roughness});

		auto F = fresnel(math::dot(-wo, wm), eta);
		auto D = trowbridge_reitz(wm);

		auto pr = F;
		auto pt = 1.f - pr;
		auto flags = this->flags();
		pr *= (flags & bsdf::Bsdf::reflective);
		pt *= (flags & bsdf::Bsdf::transmissive);
		if (pr == 0.f && pt == 0.f) {
			return {};
		}

		auto reflective = u[0] < pr / (pr + pt);
		auto wi = reflective ? math::reflect(wo, wm) : math::refract(wo, wm, eta.r);
		auto G = smith_shadow(wo, wi);

		return torrance_sparrow(reflective, pr, pt, F, D, G, wo, wi, wm);
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

		bsdf->eta = 1.f
		* math::Complex<f32>{bsdf->interior_eta.value[0], bsdf->interior_k.value[0]}
		/ math::Complex<f32>{bsdf->exterior_eta.value[0], bsdf->exterior_k.value[0]};

		return bsdf;
	}

	auto Microfacet_Bsdf::flags() const -> Flags {
		auto flags = 0;
		if (eta.i > 0.f) {
			flags |= bsdf::Bsdf::reflective;
		} else if (eta.r == 1.f) {
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
		+ math::sqr(cos_phi / u_roughness)
		+ math::sqr(sin_phi / v_roughness));

		return 1.f / (math::pi * u_roughness * v_roughness * cos4_theta * math::sqr(1.f + e));
	}

	auto Microfacet_Bsdf::visible_trowbridge_reitz(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wm) const -> f32 {
		return 1.f
		* trowbridge_reitz(wm)
		* smith_mask(-wo)
		* math::abs(math::dot(-wo, wm))
		/ math::abs(math::unit_to_cos_theta(-wo));
	}

	auto Microfacet_Bsdf::lambda(math::Vector<f32, 3> const& wo) const -> f32 {
		auto tan2_theta = math::unit_to_tan2_theta(wo);
		if (std::isinf(tan2_theta)) {
			return 0.f;
		}
		auto alpha2 = 0.f
		+ math::sqr(math::unit_to_cos_theta(wo) * u_roughness)
		+ math::sqr(math::unit_to_sin_theta(wo) * v_roughness);
		return (math::sqrt(1.f + alpha2 * tan2_theta) - 1.f) / 2.f;
	}

	auto Microfacet_Bsdf::smith_mask(math::Vector<f32, 3> const& wo) const -> f32 {
		return 1.f / (1.f + lambda(-wo));
	}

	auto Microfacet_Bsdf::smith_shadow(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wi) const -> f32 {
		return 1.f / (1.f + lambda(-wo) + lambda(wi));
	}

	auto Microfacet_Bsdf::torrance_sparrow(
		bool reflective, f32 pr, f32 pt,
		f32 F, f32 D, f32 G,
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi,
		math::Vector<f32, 3> const& wm
	) const -> std::optional<Interaction> {
		auto cos_theta_o = math::unit_to_cos_theta(-wo);
		auto cos_theta_i = math::unit_to_cos_theta(wi);
		auto cos_theta_om = math::dot(-wo, wm);
		auto cos_theta_im = math::dot(wi, wm);

		auto f = exterior_eta;
		auto pdf = 1.f;
		auto denom = 0.f;
		if (reflective) {
			f = F * D * G / math::abs(4.f * cos_theta_o * cos_theta_i);
			pdf = visible_trowbridge_reitz(wo, wm) / (4.f * math::abs(cos_theta_om)) * pr / (pr + pt);
		} else {
			denom = math::sqr(cos_theta_im + cos_theta_om / eta.r);
			f = (1.f - F) * D * G * math::abs(cos_theta_om * cos_theta_im / (denom * cos_theta_i * cos_theta_o)) / math::sqr(eta.r);
			pdf = visible_trowbridge_reitz(wo, wm) * math::abs(cos_theta_im) / denom * pt / (pr + pt);
		}

		return Interaction{f, wi, pdf};
	}
}
