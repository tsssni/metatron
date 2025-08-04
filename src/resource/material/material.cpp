#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/spectra/constant.hpp>

namespace mtt::material {
	auto Material::sample(
		eval::Context const& ctx,
		texture::Coordinate const& coord
	) const noexcept -> std::optional<Interaction> {
		auto attr = bsdf::Attribute{};
		auto intr = Interaction{};
		auto spec = ctx.spec;

		auto null_spec = spec & spectra::Spectrum::spectra["zero"];
		auto geometry_normal = math::Vector<f32, 3>{0.f, 0.f, 1.f};
		attr.spectra["spectrum"] = null_spec;
		attr.spectra["emission"] = null_spec;
		attr.vectors["normal"] = geometry_normal;

		for (auto const& [name, tex]: spectrum_textures) {
			attr.spectra[name] = tex->sample(ctx, coord);
		}

		for (auto const& [name, tex]: vector_textures) {
			attr.vectors[name] = tex->sample(ctx, coord);
		}

		intr.emission = attr.spectra["emission"];
		intr.normal = attr.vectors["normal"];

		attr.inside = ctx.inside;
		intr.bsdf = bsdf; intr.bsdf->configure(attr);
		intr.degraded = intr.bsdf->degrade();

		return intr;
	}
}
