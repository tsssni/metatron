#pragma once
#include <metatron/resource/color/color-space.hpp>

namespace mtt::compo {
	enum struct Color_Space: i32 {
		sRGB,
	};

	auto to_color_space(Color_Space color_space) -> view<color::Color_Space>;
}
