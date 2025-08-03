#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/ecs/stage.hpp>
#include <unordered_map>
#include <print>

namespace mtt::ecs {
	mut<Hierarchy> Hierarchy::instance{nullptr};

	struct Hierarchy::Impl final {
		mut<Hierarchy> hierarchy;

		std::unordered_map<std::string, Entity> entities;
		std::unordered_map<Entity, std::string> names;
		std::unordered_map<Entity, Entity> fathers;
		std::unordered_map<Entity, std::vector<Entity>> sons;

		auto plant() noexcept -> void {
			entities["/"] = hierarchy->registry.create();
			fathers[entities["/"]] = ecs::null;
		}

		auto create(std::string const& name) noexcept -> Entity {
			if (entities.find(name) != entities.end()) {
				return entities[name];
			}

			auto slash = name.find_last_of('/');
			if (slash == std::string::npos) {
				std::println("ecs: invalid name {}", name);
				std::abort();
			}
			
			auto parent_name = slash == 0 ? "/" : name.substr(0, slash);
			auto parent = create(parent_name);

			auto entity = hierarchy->registry.create();
			fathers[entity] = entities[parent_name];
			sons[parent].push_back(entity);
			sons[entity] = {};

			names[entity] = name;
			entities[name] = entity;
			return entity;
		}

		template<typename T, typename U>
		auto fetch(
			std::unordered_map<T, U> const& map, 
		    std::conditional_t<sizeof(T) <= sizeof(void*) && std::is_trivially_copyable_v<T>, T, T const&> key
		) const -> std::conditional_t<sizeof(U) <= sizeof(void*) && std::is_trivially_copyable_v<U>, U, U const&> {
			auto it = map.find(key);
			if (it != map.end()) {
				return it->second;
			} else {
				std::println("ecs: invalid key");
				std::abort();
			}
		}
	};

	Hierarchy::Hierarchy() noexcept {
		impl->hierarchy = this;
		impl->plant();
	}

	auto Hierarchy::activate() noexcept -> void {
		Hierarchy::instance = this;
	}

	auto Hierarchy::create(std::string const& name) noexcept -> Entity {
		return impl->create(name);
	}

	auto Hierarchy::entity(std::string const& name) const noexcept -> Entity {
		return impl->fetch(impl->entities, name);
	}

	auto Hierarchy::name(Entity entity) const noexcept -> std::string const& {
		return impl->fetch(impl->names, entity);
	}

	auto Hierarchy::root() const noexcept -> Entity {
		return impl->entities.at("/");
	}

	auto Hierarchy::parent(Entity entity) const noexcept -> Entity {
		return impl->fetch(impl->fathers, entity);
	}

	auto Hierarchy::children(Entity entity) const noexcept -> std::vector<Entity> const& {
		return impl->fetch(impl->sons, entity);
	}

	auto Hierarchy::init() noexcept -> void {
		for (auto& stage: stages) {
			stage->init();
		}
	}

	auto Hierarchy::update() noexcept -> void {
		for (auto& stage: stages) {
			stage->update();
		}
	}
}
