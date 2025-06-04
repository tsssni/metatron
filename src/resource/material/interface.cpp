#include <metatron/resource/material/interface.hpp>
#include <metatron/resource/spectra/constant.hpp>

namespace metatron::material {
	Interface_Material::Interface_Material() = default;

	auto Interface_Material::sample(
		eval::Context const& ctx,
		texture::Coordinate const& coord
	) const -> std::optional<Interaction> {
		return {};
	}
}
