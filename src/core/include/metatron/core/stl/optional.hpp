#pragma once

namespace metatron::stl {
	#define METATRON_OPT_OR_CALLBACK(x, opt, f)\
		auto x##_opt = opt;\
		if (!x##_opt) f\
		auto& x = x##_opt.value()

	#define METATRON_OPT_OR_BREAK(x, opt) METATRON_OPT_OR_CALLBACK(x, opt, {break;})
	#define METATRON_OPT_OR_CONTINUE(x, opt) METATRON_OPT_OR_CALLBACK(x, opt, {continue;})
	#define METATRON_OPT_OR_RETURN(x, opt, ...) METATRON_OPT_OR_CALLBACK(x, opt, {return __VA_ARGS__;})
}
