#pragma once
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::daemon {
	struct Color_Space_Daemon final {
		Color_Space_Daemon(ecs::Hierarchy& hierarchy) noexcept;
		auto update(ecs::Hierarchy& hierarchy) noexcept -> void;
	};
}
