#include "metatron/core/math/matrix.hpp"
#include <metatron/core/math/transform.hpp>

namespace metatron::math {
	Transform::Transform(
		math::Vector<f32, 3> translation,
		math::Vector<f32, 3> scaling,
		math::Quaternion<f32> rotation
	): config(translation, scaling, rotation) {
		update(true);
	}

	Transform::operator math::Matrix<f32, 4, 4>() const {
		update();
		return transform;
	}
	auto Transform::operator|(math::Vector<f32, 4> const& rhs) const -> math::Vector<f32, 4> {
		update();
		return transform | rhs;
	}

	auto Transform::operator^(math::Vector<f32, 4> const& rhs) const -> math::Vector<f32, 4> {
		update();
		return inv_transform | rhs;
	}

	auto Transform::operator|(math::Ray const& rhs) const -> math::Ray {
		auto r = rhs;
		r.o = *this | math::Vector<f32, 4>{r.o, 1.f};
		r.d = *this | math::Vector<f32, 4>{r.d, 0.f};
		return r;
	}
	auto Transform::operator^(math::Ray const& rhs) const -> math::Ray {
		auto r = rhs;
		r.o = *this ^ math::Vector<f32, 4>{r.o, 1.f};
		r.d = *this ^ math::Vector<f32, 4>{r.d, 0.f};
		return r;

	}
	auto Transform::operator|(math::Ray_Differential const& rhs) const -> math::Ray_Differential {
		auto ray = rhs;
		ray.r = *this | rhs.r;
		ray.rx = *this | rhs.rx;
		ray.ry = *this | rhs.ry;
		return ray;
	}
	auto Transform::operator^(math::Ray_Differential const& rhs) const -> math::Ray_Differential {
		auto ray = rhs;
		ray.r = *this ^ rhs.r;
		ray.rx = *this ^ rhs.rx;
		ray.ry = *this ^ rhs.ry;
		return ray;
	}

	auto Transform::update(bool force) const -> void {
		if (force || config != old_config) {
			auto translation = math::Matrix<f32, 4, 4>{
				{1.f, 0.f, 0.f, config.translation[0]},
				{0.f, 1.f, 0.f, config.translation[1]},
				{0.f, 0.f, 1.f, config.translation[2]},
				{0.f, 0.f, 0.f, 1.f}
			};
			auto scaling = math::Matrix<f32, 4, 4>{
				config.scaling[0], config.scaling[1], config.scaling[2], 1.f
			};
			auto rotation = math::Matrix<f32, 4, 4>{config.rotation};

			old_config = config;
			transform = translation | rotation | scaling;
			inv_transform = math::inverse(transform);
		}
	}
}
