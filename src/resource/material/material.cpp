#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/interface.hpp>

namespace metatron::material {
	auto Material::sample(
		eval::Context const& ctx,
		texture::Coordinate const& coord
	) const -> std::optional<Interaction> {
		auto attr = bsdf::Attribute{};
		auto intr = Interaction{};
		attr.spectrum = ctx.spec;
		attr.reflectance = reflectance->sample(ctx, coord);
		attr.transmittance = transmittance->sample(ctx, coord);
		intr.normal = nomral->sample(ctx, coord);
		intr.emission = emission->sample(ctx, coord);
		intr.bsdf = bsdf->clone(attr);
		return intr;
	}
}
