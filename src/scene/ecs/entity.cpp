#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt {
	auto operator"" _et(view<char> path, usize size) -> ecs::Entity {
		return ecs::Hierarchy::instance->entity(path);
	}
}
