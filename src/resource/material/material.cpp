#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/spectra/constant.hpp>

namespace metatron::material {
	auto Material::sample(
		eval::Context const& ctx,
		texture::Coordinate const& coord
	) const -> std::optional<Interaction> {
		auto attr = bsdf::Attribute{};
		auto intr = Interaction{};
		auto spec = ctx.spec;

		auto sample = [&](auto& attr, auto* tex, auto const& default_v) {
			if (!tex) {
				attr = default_v;
			} else {
				attr = tex->sample(ctx, coord);
			}
		};
		auto sample_spectrum = [&](auto& attr, auto* spec, auto const& default_v) {
			if (!spec) {
				attr = default_v;
			} else {
				attr = ctx.spec & *spec;
			}
		};
		auto sample_channel = [&](f32& attr, auto* tex, usize idx, auto const& default_v) {
			if (!tex) {
				attr = default_v;
			} else {
				attr = tex->sample(ctx, coord)[idx];
			}
		};

		auto null_spec = spec & spectra::Constant_Spectrum{0.f};
		auto geometry_normal = math::Vector<f32, 3>{0.f, 0.f, 1.f};

		sample(attr.reflectance, reflectance, null_spec);
		sample(attr.transmittance, transmittance, null_spec);
		sample(intr.emission, emission, null_spec);
		sample(intr.normal, nomral, geometry_normal);
		sample_channel(attr.u_roughness, u_roughness, 0, 0.f);
		sample_channel(attr.v_roughness, v_roughness, 0, 0.f);
		sample_spectrum(attr.interior_eta, interior_eta, null_spec);
		sample_spectrum(attr.exterior_eta, exterior_eta, null_spec);
		sample_spectrum(attr.interior_k, interior_k, null_spec);
		sample_spectrum(attr.exterior_k, exterior_k, null_spec);

		attr.spectrum = spec;
		attr.inside = ctx.inside;
		intr.bsdf = bsdf->clone(attr);
		intr.degraded = intr.bsdf->degrade();
		return intr;
	}
}
