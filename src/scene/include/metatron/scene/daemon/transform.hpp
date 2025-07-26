#pragma once
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::daemon {
	struct Transform_Daemon final {
		struct Impl;

		auto update(ecs::Hierarchy&) noexcept -> void;

	private:
		auto transform(ecs::Hierarchy&, ecs::Entity entity) noexcept -> void;
		auto up_trace(ecs::Hierarchy&, ecs::Entity entity) noexcept -> bool;
		auto down_trace(ecs::Hierarchy&, ecs::Entity entity) noexcept -> void;
	};
}
