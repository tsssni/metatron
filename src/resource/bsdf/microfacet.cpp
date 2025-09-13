#include <metatron/resource/bsdf/microfacet.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/disk.hpp>

namespace mtt::bsdf {
	struct Microfacet_Bsdf::Impl final {
		f32 alpha_u;
		f32 alpha_v;
		spectra::Stochastic_Spectrum eta;
		spectra::Stochastic_Spectrum k;

		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const noexcept -> std::optional<Interaction> {
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

			auto F = fresnel(math::dot(-wo, wm), eta, k);
			auto D = trowbridge_reitz(wm, alpha_u, alpha_v);
			auto G = smith_shadow(wo, wi, alpha_u, alpha_v);

			auto pr = F.value[0];
			auto pt = 1.f - F.value[0];
			auto flags = this->flags();
			pr *= bool(flags & Flags::reflective);
			pt *= bool(flags & Flags::transmissive);
			if (pr == 0.f && pt == 0.f) {
				return {};
			}

			return torrance_sparrow(
				reflective, pr, pt,
				F, D, G,
				wo, wi, wm,
				eta, alpha_u, alpha_v
			);
		}

		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 3> const& u
		) const noexcept -> std::optional<Interaction> {
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

			auto F = fresnel(math::dot(-wo, wm), eta, k);
			auto D = trowbridge_reitz(wm, alpha_u, alpha_v);

			auto pr = F.value[0];
			auto pt = 1.f - F.value[0];
			auto flags = this->flags();
			pr *= bool(flags & Flags::reflective);
			pt *= bool(flags & Flags::transmissive);
			if (pr == 0.f && pt == 0.f) {
				return {};
			}

			auto reflective = u[0] < pr / (pr + pt);
			auto wi = reflective ? math::reflect(wo, wm) : math::refract(wo, wm, eta.value[0]);
			auto G = smith_shadow(wo, wi, alpha_u, alpha_v);

			return wi == math::Vector<f32, 3>{0.f}
			? std::optional<Interaction>{}
			: torrance_sparrow(
				reflective, pr, pt,
				F, D, G,
				wo, wi, wm,
				eta, alpha_u, alpha_v
			);
		}

		auto configure(Attribute const& attr) noexcept -> void {
			auto null_spec = attr.spectra.at("spectrum") & spectra::Spectrum::spectra["zero"];
			eta = attr.spectra.count("eta") > 0 ? attr.spectra.at("eta") : null_spec;
			k = attr.spectra.count("k") > 0 ? attr.spectra.at("k") : null_spec;

			auto alpha = attr.vectors.count("alpha") > 0
			? attr.vectors.at("alpha")[0] : 0.001f;
			alpha_u = attr.vectors.count("alpha_u") > 0
			? attr.vectors.at("alpha_u")[0] : alpha;
			alpha_v = attr.vectors.count("alpha_v") > 0
			? attr.vectors.at("alpha_v")[0] : alpha;

			if (attr.inside) {
				eta.value = 1.f / eta.value;
			}
		}

		auto degrade() noexcept -> bool {
			if (spectra::max(k) == 0.f && !spectra::constant(eta)) {
				spectra::degrade(eta);
				spectra::degrade(k);
				return true;
			} else {
				return false;
			}
		}

		auto flags() const noexcept -> Flags {
			auto flags = 0;
			if (spectra::max(k) > 0.f) {
				flags |= Flags::reflective;
			} else if (spectra::constant(eta) && eta.value[0] == 1.f) {
				flags |= Flags::transmissive;
			} else {
				flags |= (Flags::transmissive | Flags::reflective);
			}
			return Flags(flags);
		}
	};

	Microfacet_Bsdf::Microfacet_Bsdf() noexcept {}

	auto Microfacet_Bsdf::operator()(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi
	) const noexcept -> std::optional<Interaction> {
		return (*impl)(wo, wi);
	}

	auto Microfacet_Bsdf::sample(
		eval::Context const& ctx,
		math::Vector<f32, 3> const& u
	) const noexcept -> std::optional<Interaction> {
		return impl->sample(ctx, u);
	}

	auto Microfacet_Bsdf::configure(Attribute const& attr) noexcept -> void {
		return impl->configure(attr);
	}

	auto Microfacet_Bsdf::flags() const noexcept -> Flags {
		return impl->flags();
	}

	auto Microfacet_Bsdf::degrade() noexcept -> bool {
		return impl->degrade();
	}

}
