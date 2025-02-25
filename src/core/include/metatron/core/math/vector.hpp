#pragma once
#include <metatron/core/math/matrix.hpp>
#include <cmath>

namespace metatron::math {
	template<typename T, usize size>
	using Vector = Matrix<T, size>;

	template<typename T, usize size>
	auto dot(Vector<T, size> const& x, Vector<T, size> const& y) -> T {
		auto result = T{};
		for (auto i = 0; i < size; i++) {
			result += x[i] * y[i];
		}
		return result;
	}

	template<typename T>
	auto cross(Vector<T, 3> const& x, Vector<T, 3> const& y) -> Vector<T, 3> {
		return {
			x[1] * y[2] - x[2] * y[1],
			x[2] * y[0] - x[0] * y[2],
			x[0] * y[1] - x[1] * y[0]
		};
	}

	template<typename T, usize size>
	requires std::floating_point<T>
	auto normalize(Vector<T, size> const& x) -> Vector<T, size> {
		return x / std::sqrt(dot(x, x));
	}

	template<typename T>
	requires std::floating_point<T>
	auto reflect(Vector<T, 3> const& in, Vector<T, 3> const& n) -> Vector<T, 3> {
		return T{2.0} * n * dot(-in, n) + in; 
	}

	template<typename T>
	requires std::floating_point<T>
	auto refract(Vector<T, 3> const& in, Vector<T, 3> const& n, T const& eta) -> Vector<T, 3> {
		auto cos_theta_i = dot(in, n);
		auto cos_2_theta_t = T{1.0} - eta * eta * (T{1.0} - cos_theta_i * cos_theta_i); 
		if (cos_2_theta_t < 0.0) return Vector<T, 3>{T{0.0}};
		return eta * in - (eta * cos_theta_i + std::sqrt(cos_2_theta_t)) * n;
	}

	template<typename T, usize size>
	requires std::floating_point<T>
	auto lerp(Vector<T, size> const& x, Vector<T, size> const& y, T const& alpha) -> Vector<T, 3> {
		return (T{1.0} - alpha) * x + alpha * y;
	}
}
