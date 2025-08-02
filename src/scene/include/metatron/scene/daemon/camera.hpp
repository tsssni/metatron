#pragma once
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::daemon {
	struct Camera_Daemon final {
		ecs::Entity camera_entity;
		auto update(ecs::Hierarchy& hierarchy) noexcept -> void;
	};
}
