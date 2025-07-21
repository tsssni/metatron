#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::ecs {
	struct Hierarchy final: stl::capsule<Hierarchy> {
		struct Impl;

		Registry registry;
		Hierarchy() noexcept;

		auto create(std::string const& name) noexcept -> Entity;
		auto entity(std::string const& name) const noexcept -> Entity;
		auto parent(Entity entity) const noexcept -> Entity;

		template<typename T>
		auto attach(Entity entity) noexcept -> void {
			registry.emplace<T>(entity);
		}

		template<typename T>
		auto mutate(Entity entity) noexcept -> T& {
			return registry.get<T>(entity);
		}

		template<typename T>
		auto fetch(Entity entity) const noexcept -> T const& {
			return registry.get<T const>(entity);
		}

		template<typename T>
		auto detach(Entity entity) noexcept -> void {
			registry.erase<T>(entity);
		}
	};
}
