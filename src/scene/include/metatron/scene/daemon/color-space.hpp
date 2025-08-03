#pragma once
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::daemon {
	struct Color_Space_Daemon final {
		auto init() noexcept -> void;
		auto update() noexcept -> void;
	};
}
