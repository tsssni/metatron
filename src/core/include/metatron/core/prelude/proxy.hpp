#pragma once
#include <proxy/proxy.h>

namespace mtt::inline prelude {
	template<pro::facade F>
	using proxy = pro::proxy<F>;

	#define MTT_PROXY_MEMBER(x, ...) PRO_DEF_MEM_DISPATCH(x, __VA_ARGS__)
	#define MTT_PROXY_FREE(x, ...) PRO_DEF_FREE_DISPATCH(x, __VA_ARGS__)
	#define MTT_PROXCY_FREE_MEMBER(x, ...) PRO_DEF_FREE_AS_MEM_DISPATCH(x, __VA_ARGS__)
}
