#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>

namespace metatron::hierarchy {
	struct Transform final {
		math::Vector<f32, 3> translation{};
		math::Vector<f32, 3> scaling{1.f};
		math::Quaternion<f32> rotation{0.f, 0.f, 0.f, 1.f};

		explicit operator math::Matrix<f32, 4, 4>() const;
	};
}
