#pragma once
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/geometry/material/texture/texture.hpp>
#include <metatron/geometry/material/bsdf/bsdf.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>
#include <memory>
#include <unordered_set>
#include <typeindex>

namespace metatron::material {
	struct Interaction final {
		std::unique_ptr<Bsdf> bsdf;
		spectra::Stochastic_Spectrum L;
	};

	struct Material {
		auto virtual sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> std::optional<Interaction> = 0;

		auto static initialize() -> void;
		auto static is_interface(Material const& material) -> bool;

	private:
		static std::unordered_set<std::type_index> interface_materials;
	};
}
