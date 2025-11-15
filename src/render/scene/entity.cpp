#include <metatron/render/scene/entity.hpp>
#include <metatron/render/scene/hierarchy.hpp>

namespace mtt::scene {
    auto to_path(Entity entity) -> std::string_view {
        return Hierarchy::instance().path(entity);
    }

    auto to_entity(std::string_view path) -> Entity {
        return Hierarchy::instance().entity(path);
    }
}

namespace mtt {
    auto operator""_et(view<char> path, usize size) -> scene::Entity {
        return scene::to_entity(path);
    }

    auto operator/(std::string_view path, Entity_Divider et) -> scene::Entity {
        return scene::to_entity(path.data());
    }

    auto operator/(scene::Entity entity, Path_Divider pt) -> std::string_view {
        return scene::to_path(entity);
    }
}
