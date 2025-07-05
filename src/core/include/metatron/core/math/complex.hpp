#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
	template <typename T>
	struct Complex {
		T r;
		T i;

		Complex() = default;
		Complex(T r, T i = T{0}): r(r), i(i) {}

		auto constexpr operator+(Complex const& rhs) const -> Complex {
			return {r + rhs.r, i + rhs.i};
		}

		auto constexpr operator+=(Complex const& rhs) -> Complex& {
			*this = *this + rhs;
			return *this;
		}

		auto constexpr operator+(T const& rhs) const -> Complex {
			return *this + Complex{rhs};
		}

		auto constexpr operator-(const Complex& rhs) const -> Complex {
			return {r - rhs.r, i - rhs.i};
		}

		auto constexpr operator-=(const Complex& rhs) -> Complex& {
			*this = *this - rhs;
			return *this;
		}

		auto constexpr operator-(T const& rhs) const -> Complex {
			return *this - Complex{rhs};
		}

		auto constexpr operator-() const -> Complex {
			return {-r, -i};
		}

		auto constexpr operator*(const Complex& rhs) const -> Complex {
			return {
				r * rhs.r - i * rhs.i,
			    r * rhs.i + i * rhs.r,
			};
		}

		auto constexpr operator*=(const Complex& rhs) -> Complex& {
			*this = *this * rhs;
			return *this;
		}

		auto constexpr operator*(T const& rhs) const -> Complex {
			return *this * Complex{rhs};
		}

		auto constexpr operator/(const Complex& rhs) const -> Complex {
			auto denom = rhs.r * rhs.r + rhs.i * rhs.i;
			return {
				math::guarded_div(r * rhs.r + i * rhs.i, denom),
			    math::guarded_div(i * rhs.r - r * rhs.i, denom),
			};
		}

		auto constexpr operator/=(const Complex& rhs) -> Complex& {
			*this = *this / rhs;
			return *this;
		}

		auto constexpr operator/(T const& rhs) const -> Complex {
			return *this / Complex{rhs};
		}

		template<usize idx>
		auto constexpr get() const -> T const& {
			static_assert(idx < 2, "index out of bounds");
			if constexpr (idx == 0) {
				return r;
			} else {
				return i;
			}
		}

		template<usize idx>
		auto constexpr get() -> T& {
			static_assert(idx < 2, "index out of bounds");
			if constexpr (idx == 0) {
				return r;
			} else {
				return i;
			}
		}
	};

	template <typename T>
	auto constexpr operator+(T const& lhs, Complex<T> const& rhs) -> Complex<T> {
		return rhs + lhs;
	}

	template <typename T>
	auto constexpr operator-(T const& lhs, Complex<T> const& rhs) -> Complex<T> {
		return -rhs + lhs;
	}

	template <typename T>
	auto constexpr operator*(T const& lhs, Complex<T> const& rhs) -> Complex<T> {
		return rhs * lhs;
	}

	template <typename T>
	auto constexpr operator/(T const& lhs, Complex<T> const& rhs) -> Complex<T> {
		return Complex<T>{lhs} / rhs;
	}

	template<usize idx, typename T>
	auto constexpr get(Complex<T> const& z)  -> T const& {
		return z.template get<idx>();
	}

	template<usize idx, typename T>
	auto constexpr get(Complex<T>& z) -> T& {
		return z.template get<idx>();
	}

	template <typename T>
	requires std::floating_point<T>
	auto constexpr norm(Complex<T> const& z) -> T {
		return z.r * z.r + z.i * z.i;
	}

	template <typename T>
	requires std::floating_point<T>
	auto constexpr abs(Complex<T> const& z) -> T {
		return math::sqrt(norm(z));
	}

	template <typename T>
	requires std::floating_point<T>
	auto constexpr sqrt(Complex<T> const& z) -> Complex<T> {
		// z = a + bi, w = u + vi = sqrt(z)
		// u^2 - v^2 = a, 2uv = b
		auto [a, b] = z;

		if (b != T{0}) {
			// v = b / 2u
			// u^4 - a u^2 - b^2 / 4 = 0
			// u^2 = (a + sqrt(a^2 + b^2)) / 2
			auto u = math::sqrt((a + abs(z)) * T{0.5});
			auto v = T{0.5} * b / u;
			return {u, v};
		} else if (a >= T{0}) {
			return {math::sqrt(a), T{0}};
		} else {
			return {T{0}, math::sqrt(-a)};
		}
	}

	template <typename T>
	requires std::floating_point<T>
	auto constexpr sqr(Complex<T> const& z) -> Complex<T> {
		return z * z;
	}
}
