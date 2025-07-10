#include <metatron/scene/ecs/hierarchy.hpp>
#include <unordered_map>

namespace mtt::ecs {
	struct Hierarchy::Impl final {
		std::unordered_map<std::string, Entity> entities;
		std::unordered_map<Entity, Entity> parents;
	};

	Hierarchy::Hierarchy() {
		impl->entities["/"] = registry.create();
		impl->parents[impl->entities["/"]] = ecs::null;
	}

	auto Hierarchy::create(std::string const& name) noexcept -> Entity {
		if (impl->entities.find(name) != impl->entities.end()) {
			return impl->entities[name];
		}

		auto slash = name.find_last_of('/');
		if (slash == std::string::npos) {
			std::printf("ecs: invalid name");
			std::abort();
		}
		
		auto parent_name = slash == 0 ? "/" : name.substr(0, slash);
		auto parent = create(parent_name);

		impl->entities[name] = registry.create();
		impl->parents[impl->entities[name]] = impl->entities[parent_name];
		return impl->entities[name];
	}

	auto Hierarchy::entity(std::string const& name) const noexcept -> Entity {
		auto it = impl->entities.find(name);
		if (it != impl->entities.end()) {
			return it->second;
		} else {
			std::printf("ecs: invalid name");
			std::abort();
		}
	}

	auto Hierarchy::parent(Entity entity) const noexcept -> Entity {
		auto it = impl->parents.find(entity);
		if (it != impl->parents.end()) {
			return it->second;
		} else {
			std::printf("ecs: invalid entity");
			std::abort();
		}
	}
}
