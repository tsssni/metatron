#pragma once
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/core/math/ray.hpp>
#include <memory>
#include <unordered_set>
#include <typeindex>

namespace metatron::material {
	struct Interaction final {
		std::unique_ptr<bsdf::Bsdf> bsdf;
		spectra::Stochastic_Spectrum L;
		math::Vector<f32, 3> n;
	};

	struct Material {
		virtual ~Material() {}
		auto virtual sample(
			eval::Context const& ctx,
			texture::Coordinate const& coord
		) const -> std::optional<Interaction> = 0;

		auto static initialize() -> void;
		auto static is_interface(Material const& material) -> bool;

	private:
		static std::unordered_set<std::type_index> interface_materials;
	};
}
