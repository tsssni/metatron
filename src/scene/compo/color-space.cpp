#include <metatron/scene/compo/color-space.hpp>

namespace mtt::compo {
	auto to_color_space(Color_Space color_space) -> view<color::Color_Space> {
		switch (color_space) {
		case Color_Space::sRGB:
			return color::Color_Space::sRGB.get();
		default:
			return color::Color_Space::sRGB.get();
		};
	}
}
