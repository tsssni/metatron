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
	auto operator"" _et(mtt::view<char> path, usize size) -> mtt::ecs::Entity;
}
