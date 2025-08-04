#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <unordered_map>

namespace mtt::compo {
	enum struct Bsdf: i32 {
		interface,
		lambertian,
		microfacet,
	};

	struct Material final {
		Bsdf bsdf;
		std::unordered_map<std::string, ecs::Entity> spectrum_textures;
		std::unordered_map<std::string, ecs::Entity> vector_textures;
	};
}
