#pragma once
#include <entt/entt.hpp>
#include <metatron/core/stl/poly_vector.hpp>

namespace mtt::scene {
    using Registry = entt::registry;
    using Entity = entt::entity;
    auto constexpr null = entt::null;

    auto to_path(Entity entity) -> std::string_view;
    auto to_entity(std::string const& path) -> Entity;
}

namespace mtt::scene {
    struct Entity_Divider final {} inline constexpr et;
    struct Path_Divider final {} inline constexpr pt;

    auto operator""_et(view<char> path, usize size) -> scene::Entity;
    auto operator/(std::string_view path, Entity_Divider et) -> scene::Entity;
    auto operator/(scene::Entity entity, Path_Divider pt) -> std::string_view;
}
