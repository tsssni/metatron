#pragma once
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::daemon {
	struct Camera_Daemon final {
		ecs::Entity camera_entity;
		auto init() noexcept -> void;
		auto update() noexcept -> void;
	};
}
