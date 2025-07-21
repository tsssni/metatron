#pragma once
#include <metatron/scene/ecs/entity.hpp>

namespace mtt::ecs {
	struct Transform_Daemon final {
		auto trigger(Registry& registry) noexcept -> void;
	};
}
