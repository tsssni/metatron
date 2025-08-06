#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <variant>

namespace mtt::compo {
	struct Mesh final {
		std::string path;
		i32 mesh{0};
	};

	struct Sphere final {
		i32 sphere{0};
	};

	using Shape = std::variant<Mesh, Sphere>;

	struct Shape_Instance final {
		ecs::Entity path;
	};
}
