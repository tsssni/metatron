#pragma once

namespace mtt::stl {
	template<typename T, bool local = false>
	struct singleton {
		singleton() = default;
		singleton(singleton const&) = delete;
		singleton(singleton&&) = delete;
		auto operator=(singleton const&) -> singleton& = delete;
		auto operator=(singleton&&) -> singleton& = delete;

		static auto instance() -> T& {
			if constexpr (local) {
				T thread_local instance;
				return instance;
			} else {
				T static instance;
				return instance;
			}
		}
	};
}
