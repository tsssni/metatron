#pragma once

namespace mtt::daemon {
	struct Light_Daemon final {
		auto init() noexcept -> void;
		auto update() noexcept -> void;
	};
}
