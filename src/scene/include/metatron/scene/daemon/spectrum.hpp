#pragma once
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::daemon {
	struct Spectrum_Daemon final {
		auto init() noexcept -> void;
		auto update() noexcept -> void;
	};
}
