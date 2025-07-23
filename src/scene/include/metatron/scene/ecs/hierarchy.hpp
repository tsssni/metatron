#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/ecs/stage.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::ecs {
	struct Hierarchy final: stl::capsule<Hierarchy> {
		struct Impl;
		Registry registry;
		std::vector<poly<Stage>> stages;
		Hierarchy() noexcept;

		auto create(std::string const& name) noexcept -> Entity;
		auto entity(std::string const& name) const noexcept -> Entity;
		auto parent(Entity entity) const noexcept -> Entity;
		auto children(Entity entity) const noexcept -> std::vector<Entity> const&;

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

		auto update() noexcept -> void;
	};
}
