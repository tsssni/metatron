#pragma once
#include <proxy/proxy.h>

namespace mtt::inline prelude {
	template<typename T>
	struct poly_impl final {
		using type = std::unique_ptr<T>;
	};

	template<pro::facade F>
	struct poly_impl<F> final {
		using type = pro::proxy<F>;
	};

	template<typename T>
	using poly = poly_impl<T>::type;

	template<typename T, typename... Args>
	auto make_poly(Args&&... args) -> poly<T> {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<pro::facade F, typename T, typename... Args>
	auto make_poly(Args&&... args) -> poly<F> {
		return pro::make_proxy<F, T>(std::forward<Args>(args)...);
	}

	template<pro::facade F, typename T>
	auto make_poly(T&& value) -> poly<F> {
		return pro::make_proxy<F, T>(std::forward<T>(value));
	}

	template<typename T>
	struct view_impl final {
		using type = std::conditional_t<
			std::is_const_v<T>,
			T const*,
			T*
		>;
	};

	template<pro::facade F>
	struct view_impl<F> final {
		using facade = std::remove_const_t<F>;
		using type = std::conditional_t<
			std::is_const_v<F>,
			pro::proxy_view<facade> const,
			pro::proxy_view<facade>
		>;
	};

	template<typename T>
	using mut = view_impl<std::remove_cv_t<T>>::type;

	template<typename T>
	using view = view_impl<std::add_const_t<std::remove_cv_t<T>>>::type;

	#define MTT_POLY_METHOD(x, ...) PRO_DEF_MEM_DISPATCH(x, __VA_ARGS__)
	#define MTT_POLY_FUNCTION(x, ...) PRO_DEF_FREE_DISPATCH(x, __VA_ARGS__)
	#define MTT_POLY_FUNCTION_METHOD(x, ...) PRO_DEF_FREE_AS_MEM_DISPATCH(x, __VA_ARGS__)
}
