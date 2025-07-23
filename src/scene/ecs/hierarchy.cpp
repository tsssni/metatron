#include <metatron/scene/ecs/hierarchy.hpp>
#include <unordered_map>

namespace mtt::ecs {
	struct Hierarchy::Impl final {
		mut<Registry> registry;
		std::unordered_map<std::string, Entity> entities;
		std::unordered_map<Entity, Entity> parents;

		auto trigger() noexcept -> void {
		}

		auto plant() noexcept -> void {
			entities["/"] = registry->create();
			parents[entities["/"]] = ecs::null;
		}

		auto create(std::string const& name) noexcept -> Entity {
			if (entities.find(name) != entities.end()) {
				return entities[name];
			}

			auto slash = name.find_last_of('/');
			if (slash == std::string::npos) {
				std::printf("ecs: invalid name");
				std::abort();
			}
			
			auto parent_name = slash == 0 ? "/" : name.substr(0, slash);
			auto parent = create(parent_name);

			entities[name] = registry->create();
			parents[entities[name]] = entities[parent_name];
			return entities[name];
		}

		auto entity(std::string const& name) const noexcept -> Entity {
			auto it = entities.find(name);
			if (it != entities.end()) {
				return it->second;
			} else {
				std::printf("ecs: invalid name");
				std::abort();
			}
		}

		auto parent(Entity entity) const noexcept -> Entity {
			auto it = parents.find(entity);
			if (it != parents.end()) {
				return it->second;
			} else {
				std::printf("ecs: invalid entity");
				std::abort();
			}
		}
	};

	Hierarchy::Hierarchy() noexcept {
		impl->registry = &registry;
		impl->trigger();
		impl->plant();
	}

	auto Hierarchy::create(std::string const& name) noexcept -> Entity {
		return impl->create(name);
	}

	auto Hierarchy::entity(std::string const& name) const noexcept -> Entity {
		return impl->entity(name);
	}

	auto Hierarchy::parent(Entity entity) const noexcept -> Entity {
		return impl->parent(entity);
	}
}
