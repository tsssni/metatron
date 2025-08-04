#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::daemon {
	struct Camera_Daemon final {
		ecs::Entity camera_entity;
		math::Transform world_to_render;
		math::Transform render_to_camera;
		auto init() noexcept -> void;
		auto update() noexcept -> void;
	};
}
