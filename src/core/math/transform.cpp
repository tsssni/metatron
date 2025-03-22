#include "metatron/core/math/matrix.hpp"
#include <metatron/core/math/transform.hpp>

namespace metatron::math {
	Transform::operator math::Matrix<f32, 4, 4>() const {
		auto translation = math::Matrix<f32, 4, 4>{
			{1.f, 0.f, 0.f, this->translation[0]},
			{0.f, 1.f, 0.f, this->translation[1]},
			{0.f, 0.f, 1.f, this->translation[2]},
			{0.f, 0.f, 0.f, 1.f}
		};
		auto scaling = math::Matrix<f32, 4, 4>{
			this->scaling[0], this->scaling[1], this->scaling[2], 1.f
		};
		auto rotation = math::Matrix<f32, 4, 4>{this->rotation};
		return translation | rotation | scaling;
	}
	auto Transform::operator|(math::Vector<f32, 4> const& rhs) const -> math::Vector<f32, 4> {
		return math::Matrix<f32, 4, 4>{*this} | rhs;
	}

	auto Transform::operator^(math::Vector<f32, 4> const& rhs) const -> math::Vector<f32, 4> {
		return math::inverse(math::Matrix<f32, 4, 4>{*this}) | rhs;
	}
}
