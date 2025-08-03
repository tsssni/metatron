#pragma once
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::daemon {
	struct Transform_Daemon final {
		auto init() noexcept -> void;
		auto update() noexcept -> void;

	private:
		auto transform(ecs::Entity entity) noexcept -> void;
		auto up_trace(ecs::Entity entity) noexcept -> bool;
		auto down_trace(ecs::Entity entity) noexcept -> void;
	};
}
