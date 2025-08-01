#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <variant>

namespace mtt::compo {
	struct Mesh final {
		std::string path;
	};

	struct Sphere final {};

	using Shape = std::variant<Mesh, Sphere>;

	struct Shape_Instance final {
		ecs::Entity path;
	};
}
