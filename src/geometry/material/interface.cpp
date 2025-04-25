#include <metatron/geometry/material/interface.hpp>
#include <metatron/geometry/material/bsdf/cross.hpp>
#include <metatron/core/spectra/constant.hpp>

namespace metatron::material {
	Interface_Material::Interface_Material() = default;

	auto Interface_Material::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const -> std::optional<Interaction> {
		return Interaction{
			std::make_unique<Cross_Bsdf>(),
			ctx.L & spectra::Constant_Spectrum{0.f},
		};
	}
}
