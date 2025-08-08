#pragma once
#include <metatron/core/math/transform.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <variant>

namespace mtt::compo {
	struct Local_Transform final {
		math::Vector<f32, 3> translation{0.f};
		math::Vector<f32, 3> scaling{1.f};
		math::Quaternion<f32> rotation{0.f, 0.f, 0.f, 1.f};
	};

	struct Matrix_Transform final {
		math::Matrix<f32, 4, 4> matrix{1.f};
	};

	using Transform = std::variant<
		Local_Transform,
		Matrix_Transform
	>;

	auto to_transform(Transform const& t) -> math::Transform;
}
