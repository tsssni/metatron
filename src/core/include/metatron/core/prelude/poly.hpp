#pragma once
#include <proxy/proxy.h>

namespace mtt::inline prelude {
	template<pro::facade F>
	using poly = pro::proxy<F>;

	template<pro::facade F>
	using view = pro::proxy_view<F>;

	#define MTT_POLY_METHOD(x, ...) PRO_DEF_MEM_DISPATCH(x, __VA_ARGS__)
	#define MTT_POLY_FUNCTION(x, ...) PRO_DEF_FREE_DISPATCH(x, __VA_ARGS__)
	#define MTT_POLY_FUNCTION_METHOD(x, ...) PRO_DEF_FREE_AS_MEM_DISPATCH(x, __VA_ARGS__)
}
