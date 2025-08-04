#pragma once
#include <metatron/core/math/transform.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>

namespace mtt::compo {
	struct Transform final {
		math::Vector<f32, 3> translation{0.f};
		math::Vector<f32, 3> scaling{1.f};
		math::Quaternion<f32> rotation{0.f, 0.f, 0.f, 1.f};

		operator math::Transform() const;
	};
}
