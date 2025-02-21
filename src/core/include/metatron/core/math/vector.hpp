#pragma once
#include <metatron/core/math/matrix.hpp>

namespace metatron::math {
	template<typename T, usize size>
	using Vector = Matrix<T, size>;

	using Vector2 = Vector<f32, 2>;
	using Vector3 = Vector<f32, 3>;
	using Vector4 = Vector<f32, 4>;
}
