#pragma once
#include <entt/entt.hpp>

namespace mtt::ecs {
	using Registry = entt::registry;
	using Entity = entt::entity;
	auto constexpr null = entt::null;

	auto to_path(Entity entity) -> std::string;
	auto to_entity(std::string const& path) -> Entity;
}

namespace mtt {
	struct Entity_Divider final {} inline constexpr et;
	struct Path_Divider final {} inline constexpr pt;

	auto operator"" _et(mtt::view<char> path, usize size) -> mtt::ecs::Entity;
	auto operator/(std::string_view path, Entity_Divider et) -> ecs::Entity;
	auto operator/(ecs::Entity entity, Path_Divider pt) -> std::string;
}
