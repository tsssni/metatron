#pragma once

namespace mtt::stl {
	template<typename T, bool local = false>
	struct Singleton {
		Singleton() = default;
		Singleton(Singleton const&) = delete;
		Singleton(Singleton&&) = delete;
		auto operator=(Singleton const&) -> Singleton& = delete;
		auto operator=(Singleton&&) -> Singleton& = delete;

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
