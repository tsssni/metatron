#include <metatron/resource/bsdf/microfacet.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/disk.hpp>

namespace mtt::bsdf {
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

		auto conductive = spectra::max(k) > math::epsilon<f32>;
		auto reflective = -wo[1] * wi[1] > 0.f;
		if (conductive && !reflective) {
			return {};
		}

		auto wm = math::normalize(reflective ? -wo + wi : -wo + wi * eta.value[0]);
		if (wm[1] < 0.f) {
			wm *= -1.f;
		}
		if (false
		|| math::abs(wm[1]) < math::epsilon<f32>
		|| math::dot(-wo, wm) < 0.f
		|| math::dot((reflective ? 1.f : -1.f) * wi, wm) < 0.f) {
			return {};
		}

		auto F = fresnel(math::dot(-wo, wm));
		auto D = trowbridge_reitz(wm);
		auto G = smith_shadow(wo, wi);

		auto pr = F.value[0];
		auto pt = 1.f - F.value[0];
		auto flags = this->flags();
		pr *= bool(flags & bsdf::Bsdf::reflective);
		pt *= bool(flags & bsdf::Bsdf::transmissive);
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
		auto wy = math::normalize(-wo * math::Vector<f32, 3>{alpha_u, 1.f, alpha_v});
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
		wm = math::normalize(wm * math::Vector<f32, 3>{alpha_u, 1.f, alpha_v});

		auto F = fresnel(math::dot(-wo, wm));
		auto D = trowbridge_reitz(wm);

		auto pr = F.value[0];
		auto pt = 1.f - F.value[0];
		auto flags = this->flags();
		pr *= bool(flags & bsdf::Bsdf::reflective);
		pt *= bool(flags & bsdf::Bsdf::transmissive);
		if (pr == 0.f && pt == 0.f) {
			return {};
		}

		auto reflective = u[0] < pr / (pr + pt);
		auto wi = reflective ? math::reflect(wo, wm) : math::refract(wo, wm, eta.value[0]);
		auto G = smith_shadow(wo, wi);

		return wi == math::Vector<f32, 3>{0.f}
		? std::optional<Interaction>{}
		: torrance_sparrow(reflective, pr, pt, F, D, G, wo, wi, wm);
	}

	auto Microfacet_Bsdf::clone(Attribute const& attr) const -> std::unique_ptr<Bsdf> {
		auto bsdf = std::make_unique<Microfacet_Bsdf>();
		auto null_spec = attr.spectra.at("spectrum") & spectra::Constant_Spectrum{0.f};
		bsdf->eta = attr.spectra.count("eta") > 0 ? attr.spectra.at("eta") : null_spec;
		bsdf->k = attr.spectra.count("k") > 0 ? attr.spectra.at("k") : null_spec;

		auto alpha = attr.vectors.count("alpha") > 0
		? attr.vectors.at("alpha")[0] : 1.f;
		bsdf->alpha_u = attr.vectors.count("alpha_u") > 0
		? attr.vectors.at("alpha_u")[0] : alpha;
		bsdf->alpha_v = attr.vectors.count("alpha_v") > 0
		? attr.vectors.at("alpha_v")[0] : alpha;

		if (attr.inside) {
			bsdf->eta.value = 1.f / bsdf->eta.value;
		}

		return bsdf;
	}

	auto Microfacet_Bsdf::degrade() -> bool {
		if (spectra::max(k) == 0.f && !spectra::constant(eta)) {
			spectra::degrade(eta);
			spectra::degrade(k);
			return true;
		} else {
			return false;
		}
	}

	auto Microfacet_Bsdf::flags() const -> Flags {
		auto flags = 0;
		if (spectra::max(k) > 0.f) {
			flags |= bsdf::Bsdf::reflective;
		} else if (spectra::constant(eta) && eta.value[0] == 1.f) {
			flags |= bsdf::Bsdf::transmissive;
		} else {
			flags |= (bsdf::Bsdf::transmissive | bsdf::Bsdf::reflective);
		}
		return Flags(flags);
	}


	auto Microfacet_Bsdf::fresnel(f32 cos_theta_i) const -> spectra::Stochastic_Spectrum {
		auto F = eta;
		F.value = math::foreach([&](f32 lambda, usize i) {
			auto eta_k = math::Complex<f32>{eta.value[i], k.value[i]};
			cos_theta_i = std::clamp(cos_theta_i, -1.f, 1.f);
			auto sin2_theta_i = std::max(0.f, 1.f - cos_theta_i * cos_theta_i);
			auto sin2_theta_t = sin2_theta_i / (eta_k * eta_k);
			auto cos_theta_t = math::sqrt(1.f - sin2_theta_t);

			auto conductive = eta_k.i > math::epsilon<f32>;
			if (!conductive && sin2_theta_t.r >= 1.f) {
				return 1.f;
			}

			auto r_parl = 1.f
			* (eta_k * cos_theta_i - cos_theta_t)
			/ (eta_k * cos_theta_i + cos_theta_t);
			auto r_perp = 1.f
			* (cos_theta_i - eta_k * cos_theta_t)
			/ (cos_theta_i + eta_k * cos_theta_t);
			return (math::norm(r_parl) + math::norm(r_perp)) / 2.f;
		}, F.lambda);
		return F;
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
		+ math::sqr(cos_phi / alpha_u)
		+ math::sqr(sin_phi / alpha_v));

		return 1.f / (math::pi * alpha_u * alpha_v * cos4_theta * math::sqr(1.f + e));
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
		+ math::sqr(math::unit_to_cos_theta(wo) * alpha_u)
		+ math::sqr(math::unit_to_sin_theta(wo) * alpha_v);
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
		spectra::Stochastic_Spectrum const& F, f32 D, f32 G,
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi,
		math::Vector<f32, 3> const& wm
	) const -> std::optional<Interaction> {
		auto cos_theta_o = math::unit_to_cos_theta(-wo);
		auto cos_theta_i = math::unit_to_cos_theta(wi);
		auto cos_theta_om = math::dot(-wo, wm);
		auto cos_theta_im = math::dot(wi, wm);

		auto f = F;
		auto pdf = 1.f;
		auto denom = 0.f;
		if (reflective) {
			f = F * D * G / math::abs(4.f * cos_theta_o * cos_theta_i);
			pdf = visible_trowbridge_reitz(wo, wm) / (4.f * math::abs(cos_theta_om)) * pr / (pr + pt);
		} else {
			denom = math::sqr(cos_theta_im + cos_theta_om / eta.value[0]);
			f = (1.f - F) * D * G * math::abs(cos_theta_om * cos_theta_im / (denom * cos_theta_i * cos_theta_o)) / math::sqr(eta.value[0]);
			pdf = visible_trowbridge_reitz(wo, wm) * math::abs(cos_theta_im) / denom * pt / (pr + pt);
		}

		return Interaction{f, wi, pdf};
	}
}
