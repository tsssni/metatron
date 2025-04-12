#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/ray.hpp>

namespace metatron::math {
	template<typename T>
	concept Transformable = false
	|| std::is_same_v<std::remove_cvref_t<T>, math::Vector<f32, 4>>
	|| std::is_same_v<std::remove_cvref_t<T>, math::Ray>
	|| std::is_same_v<std::remove_cvref_t<T>, math::Ray_Differential>;

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

		template<Transformable T>
		auto operator|(T&& rhs) const -> std::remove_cvref_t<T> {
			if constexpr (std::is_convertible_v<T, math::Vector<f32, 4>>) {
				update();
				return transform | rhs;
			} else if constexpr (std::is_convertible_v<T, math::Ray>) {
				auto r = rhs;
				r.o = *this | math::expand(r.o, 1.f);
				r.d = *this | math::expand(r.d, 0.f);
				return r;
			} else if constexpr (std::is_convertible_v<T, math::Ray_Differential>) {
				auto ray = rhs;
				ray.r = *this | rhs.r;
				ray.rx = *this | rhs.rx;
				ray.ry = *this | rhs.ry;
				return ray;
			}
		}

		template<Transformable T>
		auto operator^(T&& rhs) const -> std::remove_cvref_t<T> {
			if constexpr (std::is_convertible_v<T, math::Vector<f32, 4>>) {
				update();
				return inv_transform | rhs;
			} else if constexpr (std::is_convertible_v<T, math::Ray>) {
				auto r = rhs;
				r.o = *this ^ math::expand(r.o, 1.f);
				r.d = *this ^ math::expand(r.d, 0.f);
				return r;
			} else if constexpr (std::is_convertible_v<T, math::Ray_Differential>) {
				auto ray = rhs;
				ray.r = *this ^ rhs.r;
				ray.rx = *this ^ rhs.rx;
				ray.ry = *this ^ rhs.ry;
				return ray;
			}
		}

	private:
		auto update(bool force = false) const -> void;

		mutable Config old_config{};
	};
}
