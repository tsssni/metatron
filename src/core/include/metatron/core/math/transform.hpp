#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/ray.hpp>

namespace metatron::math {
	struct Transform final {
		struct Config {
			math::Vector<f32, 3> translation{};
			math::Vector<f32, 3> scaling{1.f};
			math::Quaternion<f32> rotation{0.f, 0.f, 0.f, 1.f};

			auto operator<=>(Config const& rhs) const = default;
		} config;

		mutable math::Matrix<f32, 4, 4> transform;
		mutable math::Matrix<f32, 4, 4> inv_transform;

		Transform(
			math::Vector<f32, 3> translation = {},
			math::Vector<f32, 3> scaling = {1.f},
			math::Quaternion<f32> rotation = {0.f, 0.f, 0.f, 1.f}
		);

		explicit operator math::Matrix<f32, 4, 4>() const;
		auto operator|(math::Vector<f32, 4> const& rhs) const -> math::Vector<f32, 4>;
		auto operator^(math::Vector<f32, 4> const& rhs) const -> math::Vector<f32, 4>;
		auto operator|(math::Ray const& rhs) const -> math::Ray;
		auto operator^(math::Ray const& rhs) const -> math::Ray;
		auto operator|(math::Ray_Differential const& rhs) const -> math::Ray_Differential;
		auto operator^(math::Ray_Differential const& rhs) const -> math::Ray_Differential;

	private:
		auto update(bool force = false) const -> void;

		mutable Config old_config{};
	};
}
