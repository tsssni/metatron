#pragma once

namespace metatron::stl {
	#define OPTIONAL_BREAK_CALLBACK(x, o, f, ...)\
		auto x##_opt = o;\
		if (!x##_opt) {\
			f(__VA_ARGS__);\
			break;\
		}\
		auto& x = x##_opt.value();

	#define OPTIONAL_BREAK(x, o) OPTIONAL_BREAK_CALLBACK(x, o, void)

	#define OPTIONAL_CONTINUE_CALLBACK(x, o, f, ...)\
		auto x##_opt = o;\
		if (!x##_opt) {\
			f(__VA_ARGS__);\
			break;\
		}\
		auto& x = x##_opt.value();

	#define OPTIONAL_CONTINUE(x, o) OPTIONAL_BREAK_CALLBACK(x, o, void)

}
