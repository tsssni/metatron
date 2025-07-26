#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/ecs/stage.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::ecs {
	template<typename T>
	struct Dirty_Mark final {};

	struct Hierarchy final: stl::capsule<Hierarchy> {
		Registry registry;
		std::vector<poly<Stage>> stages;

		struct Impl;
		Hierarchy() noexcept;

		auto create(std::string const& name) noexcept -> Entity;
		auto entity(std::string const& name) const noexcept -> Entity;
		auto name(Entity entity) const noexcept -> std::string const&;

		auto root() const noexcept -> Entity;
		auto parent(Entity entity) const noexcept -> Entity;
		auto children(Entity entity) const noexcept -> std::vector<Entity> const&;

		auto update() noexcept -> void;

		template<typename T>
		auto attach(Entity entity, T&& component = {}) noexcept -> void {
			registry.emplace<Dirty_Mark<T>>(entity);
			registry.emplace<T>(entity, std::forward<T>(component));
		}

		template<typename T>
		auto mutate(Entity entity) noexcept -> T& {
			registry.emplace<Dirty_Mark<T>>(entity);
			return registry.get<T>(entity);
		}

		template<typename T>
		auto fetch(Entity entity) const noexcept -> T const& {
			return registry.get<T const>(entity);
		}

		template<typename T>
		auto detach(Entity entity) noexcept -> void {
			registry.emplace<Dirty_Mark<T>>(entity);
			registry.erase<T>(entity);
		}
	};
}
