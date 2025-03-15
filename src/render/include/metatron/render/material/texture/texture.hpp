#pragma once
#include <metatron/geometry/shape/sphere.hpp>

namespace metatron::material {
	namespace texture {
		template<typename T>
		struct Interaction {
			T tv;
		};
	}

	template<typename T>
	struct Texture {
		using Interaction = texture::Interaction<T>;
		auto virtual operator()(math::Vector<f32, 2> const& uv) -> std::optional<Interaction> = 0;
		auto virtual sample(shape::Interaction const& intr) -> std::optional<Interaction> = 0;
	};
}
