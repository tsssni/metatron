#include <metatron/scene/daemon/transform.hpp>
#include <metatron/scene/compo/transform.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::daemon {
	auto Transform_Daemon::init() noexcept -> void {
		MTT_SERDE(Transform);
	}

	auto Transform_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		auto transform_view = registry.view<ecs::Dirty_Mark<compo::Transform>>();
		for (auto entity: transform_view) {
			if (registry.any_of<ecs::Dirty_Mark<math::Transform>>(entity)) {
				continue;
			}

			auto dirty = !registry.any_of<compo::Transform>(entity);
			if (dirty) {
				if (registry.any_of<math::Transform>(entity)) {
					registry.remove<math::Transform>(entity);
				}
				registry.emplace<ecs::Dirty_Mark<math::Transform>>(entity);
			} else {
				dirty = up_trace(entity);
			}

			if (dirty) {
				for (auto child: hierarchy.children(entity)) {
					down_trace(entity);
				}
			}
		}
		registry.clear<ecs::Dirty_Mark<compo::Transform>>();
	}

	auto Transform_Daemon::transform(ecs::Entity entity) noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		auto parent = hierarchy.parent(entity);
		if (!registry.any_of<math::Transform>(entity)) {
			registry.emplace<math::Transform>(entity);
		}

		auto const& pt = registry.any_of<math::Transform>(parent)
		? registry.get<math::Transform>(parent)
		: compo::to_transform(compo::Local_Transform{});
		auto mat = math::Matrix<f32, 4, 4>{pt | compo::to_transform(registry.get<compo::Transform>(entity))};

		registry.replace<math::Transform>(entity, math::Transform{mat});
		registry.emplace<ecs::Dirty_Mark<math::Transform>>(entity);
	}

	auto Transform_Daemon::up_trace(ecs::Entity entity) noexcept -> bool {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		auto transformed = registry.any_of<ecs::Dirty_Mark<math::Transform>>(entity);
		auto attached = registry.any_of<compo::Transform>(entity);
		auto dirty = registry.any_of<ecs::Dirty_Mark<compo::Transform>>(entity);
		if (transformed || !attached) {
			return dirty || transformed;
		}

		auto parent = hierarchy.parent(entity);
		dirty |= up_trace(parent);
		if (dirty) {
			transform(entity);
		}
		return dirty;
	}

	auto Transform_Daemon::down_trace(ecs::Entity entity) noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		if (!registry.any_of<compo::Transform>(entity)) {
			return;
		}

		transform(entity);
		for (auto child: hierarchy.children(entity)) {
			down_trace(child);
		}
	}
}
