#pragma once

namespace mtt::daemon {
	struct Texture_Daemon final {
		auto init() noexcept -> void;
		auto update() noexcept -> void;
	};
}
