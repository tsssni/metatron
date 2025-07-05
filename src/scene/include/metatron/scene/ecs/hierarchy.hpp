#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/ecs/daemon/daemon.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::ecs {
	struct Hierarchy final: stl::Capsule<Hierarchy> {
		struct Impl;
		Hierarchy();

		template<typename T>
		auto monitor(Daemon& daemon) -> void {
			registry.on_construct<T>().connect([this, &daemon](Registry &reg, Entity entity) {
				daemon.attach(*this, entity);
			});
			registry.on_update<T>().connect([this, &daemon](Registry &reg, Entity entity) {
				daemon.mutate(*this, entity);
			});
			registry.on_destroy<T>().connect([this, &daemon](Registry &reg, Entity entity) {
				daemon.detach(*this, entity);
			});
		};

		auto create(std::string const& name) -> Entity;
		auto entity(std::string const& name) const -> Entity;
		auto parent(Entity entity) const -> Entity;

		template<typename T>
		auto attach(Entity entity) -> void {
			registry.emplace<T>(entity);
		}

		template<typename T>
		auto mutate(Entity entity) -> T& {
			return registry.get<T>(entity);
		}

		template<typename T>
		auto fetch(Entity entity) const -> T const& {
			return registry.get<T const>(entity);
		}

		template<typename T>
		auto detach(Entity entity) -> void {
			registry.erase<T>(entity);
		}

	private:
		friend Daemon;
		Registry registry;
	};
}
