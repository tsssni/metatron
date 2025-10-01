#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::ecs {
    auto to_path(Entity entity) -> std::string {
        return Hierarchy::instance->path(entity);
    }

    auto to_entity(std::string const& path) -> Entity {
        return Hierarchy::instance->create(path);
    }
}

namespace mtt {
    auto operator"" _et(view<char> path, usize size) -> ecs::Entity {
        return ecs::to_entity(path);
    }

    auto operator/(std::string_view path, Entity_Divider et) -> ecs::Entity {
        return ecs::to_entity(path.data());
    }

    auto operator/(ecs::Entity entity, Path_Divider pt) -> std::string {
        return ecs::to_path(entity);
    }
}
