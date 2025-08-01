#pragma once
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::daemon {
	struct Shape_Daemon final {
		auto update(ecs::Hierarchy&) noexcept -> void;
	};
}
