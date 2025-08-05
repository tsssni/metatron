#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <variant>

namespace mtt::compo {
	struct Mesh final {
		std::string_view mesh = "";
		std::string path;
	};

	struct Sphere final {
		std::string_view sphere = "";
	};

	using Shape = std::variant<Mesh, Sphere>;

	struct Shape_Instance final {
		ecs::Entity path;
	};
}
