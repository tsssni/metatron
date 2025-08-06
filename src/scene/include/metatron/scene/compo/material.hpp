#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <unordered_map>

namespace mtt::compo {
	struct Interface_Bsdf final {
		i32 interface = 0;
	};

	struct Lambertian_Bsdf final {
		i32 lambertian = 0;
	};

	struct Microfacet_Bsdf final {
		i32 microfacet = 0;
	};

	using Bsdf = std::variant<
		Interface_Bsdf,
		Lambertian_Bsdf,
		Microfacet_Bsdf
	>;

	struct Material final {
		Bsdf bsdf;
		std::unordered_map<std::string, ecs::Entity> spectrum_textures;
		std::unordered_map<std::string, ecs::Entity> vector_textures;
	};
}
