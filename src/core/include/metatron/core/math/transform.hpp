#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/eval/context.hpp>

namespace metatron::math {
	template<typename T>
	concept Transformable = false
	|| std::is_same_v<std::remove_cvref_t<T>, Vector<f32, 4>>
	|| std::is_same_v<std::remove_cvref_t<T>, Ray>
	|| std::is_same_v<std::remove_cvref_t<T>, Ray_Differential>
	|| std::is_same_v<std::remove_cvref_t<T>, eval::Context>;

	struct Transform final {
		struct Config {
			Vector<f32, 3> translation{};
			Vector<f32, 3> scaling{1.f};
			Quaternion<f32> rotation{0.f, 0.f, 0.f, 1.f};

			auto operator<=>(Config const& rhs) const = default;
		} config;

		mutable Matrix<f32, 4, 4> transform;
		mutable Matrix<f32, 4, 4> inv_transform;

		Transform(
			Vector<f32, 3> translation = {},
			Vector<f32, 3> scaling = {1.f},
			Quaternion<f32> rotation = {0.f, 0.f, 0.f, 1.f}
		);

		explicit operator Matrix<f32, 4, 4>() const;

		template<Transformable T>
		auto operator|(T&& rhs) const -> std::remove_cvref_t<T> {
			if constexpr (std::is_convertible_v<T, Vector<f32, 4>>) {
				update();
				return transform | rhs;
			} else if constexpr (std::is_convertible_v<T, Ray>) {
				auto r = rhs;
				r.o = *this | expand(r.o, 1.f);
				r.d = *this | expand(r.d, 0.f);
				return r;
			} else if constexpr (std::is_convertible_v<T, Ray_Differential>) {
				auto ray = rhs;
				ray.r = *this | rhs.r;
				ray.rx = *this | rhs.rx;
				ray.ry = *this | rhs.ry;
				return ray;
			} else if constexpr (std::is_convertible_v<T, eval::Context>) {
				auto ctx = rhs;
				ctx.r = *this | ctx.r;
				ctx.n = *this | expand(ctx.n, 0.f);
				return ctx;
			}
		}

		template<Transformable T>
		auto operator^(T&& rhs) const -> std::remove_cvref_t<T> {
			if constexpr (std::is_convertible_v<T, Vector<f32, 4>>) {
				update();
				return inv_transform | rhs;
			} else if constexpr (std::is_convertible_v<T, Ray>) {
				auto r = rhs;
				r.o = *this ^ expand(r.o, 1.f);
				r.d = *this ^ expand(r.d, 0.f);
				return r;
			} else if constexpr (std::is_convertible_v<T, Ray_Differential>) {
				auto ray = rhs;
				ray.r = *this ^ rhs.r;
				ray.rx = *this ^ rhs.rx;
				ray.ry = *this ^ rhs.ry;
				return ray;
			} else if constexpr (std::is_convertible_v<T, eval::Context>) {
				auto ctx = rhs;
				ctx.r = *this ^ ctx.r;
				ctx.n = *this ^ expand(ctx.n, 0.f);
				return ctx;
			}
		}

	private:
		auto update(bool force = false) const -> void;

		mutable Config old_config{};
	};
}
