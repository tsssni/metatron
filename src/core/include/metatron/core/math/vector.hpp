#pragma once
#include <metatron/core/math/matrix.hpp>

namespace metatron::math {
	template<typename T, usize size>
	using Vector = Matrix<T, size>;

	template<typename T, usize size>
	auto dot(math::Vector<T, size> const& x, math::Vector<T, size> const& y) {
		auto result = T{};
		for (auto i = 0; i < size; i++) {
			result += x[i] * y[i];
		}
		return result;
	}

	template<typename T>
	auto cross(math::Vector<T, 3> const& x, math::Vector<T, 3> const& y) {
		return Vector<T, 3>{
			x[1] * y[2] - x[2] * y[1],
			x[2] * y[0] - x[0] * y[2],
			x[0] * y[1] - x[1] * y[0]
		};
	}

	template<typename T, usize size>
	requires std::floating_point<T>
	auto normalize(math::Vector<T, size> const& x) {
		return x / std::sqrt(dot(x, x));
	}

	template<typename T>
	requires std::floating_point<T>
	auto reflect(math::Vector<T, 3> const& in, math::Vector<T, 3> const& n) {
		return T{2.0} * n * dot(-in, n) + in; 
	}

	template<typename T>
	requires std::floating_point<T>
	auto refract(math::Vector<T, 3> const& in, math::Vector<T, 3> const& n, T const& eta) {
		auto cos_theta_i = dot(in, n);
		auto cos_2_theta_t = T{1.0} - eta * eta * (T{1.0} - cos_theta_i * cos_theta_i); 
		if (cos_2_theta_t < 0.0) return Vector<T, 3>{T{0.0}};
		return eta * in - (eta * cos_theta_i + std::sqrt(cos_2_theta_t)) * n;
	}

	template<typename T, usize size>
	requires std::floating_point<T>
	auto lerp(math::Vector<T, size> const& x, math::Vector<T, size> const& y, T const& alpha) {
		return (T{1.0} - alpha) * x + alpha * y;
	}
}
