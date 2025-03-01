#include <metatron/core/math/vector.hpp>

namespace metatron::render {
	struct Ray final {
		math::Vector<f32, 3> o{};
		math::Vector<f32, 3> d{};
		f32 t;
	};
}
