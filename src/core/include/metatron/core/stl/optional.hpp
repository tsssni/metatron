#pragma once

namespace metatron::stl {
	#define OPTIONAL_OR_CALLBACK(x, opt, f)\
		auto x##_opt = opt;\
		if (!x##_opt) f\
		auto& x = x##_opt.value()

	#define OPTIONAL_OR_BREAK(x, opt) OPTIONAL_OR_CALLBACK(x, opt, {break;})
	#define OPTIONAL_OR_CONTINUE(x, opt) OPTIONAL_OR_CALLBACK(x, opt, {continue;})
	#define OPTIONAL_OR_RETURN(x, opt, ...) OPTIONAL_OR_CALLBACK(x, opt, {return __VA_ARGS__;})
}
