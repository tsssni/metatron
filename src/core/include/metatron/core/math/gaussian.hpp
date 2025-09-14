#pragma once
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/polynomial.hpp>

namespace mtt::math {
	// erf fits data: https://github.com/mitsuba-renderer/drjit/blob/master/include/drjit/math.h
	template<typename T>
	requires std::floating_point<T>
	auto erf(T x) noexcept -> T {
		auto c0 = T{};
		auto c1 = T{};
		auto xa = math::abs(x);
		auto x2 = math::sqr(x);
		if constexpr (std::is_same_v<T, f32>) {
			c0 = polynomial(x2, std::to_array<T>({
				0x1.20dd74p+0, -0x1.812672p-2,
				0x1.ce0934p-4, -0x1.b5a334p-6,
				0x1.4246b4p-8, -0x1.273facp-11
			}));
			c1 = polynomial(xa, std::to_array<T>({
				-0x1.a0d71ap+0, -0x1.d51e3ap-1,
				-0x1.3a904cp-3,  0x1.1c395cp-5,
				-0x1.6856bep-8,  0x1.180f1ep-11,
				-0x1.8ca9f6p-16
			}));
		} else {
			c0 = polynomial(x2, std::to_array<T>({
				0x1.20dd750429b6dp+0,  -0x1.812746b0379bcp-2,
				0x1.ce2f21a040d12p-4,  -0x1.b82ce311fa924p-6,
				0x1.565bccf92b298p-8,  -0x1.c02db03dd71b8p-11,
				0x1.f9a2baa8fee07p-14, -0x1.f4ca4d6f3e31bp-17,
				0x1.b97fd3d992af4p-20, -0x1.5c0726f04e805p-23,
				0x1.d71b0f1b15b0ap-27, -0x1.abae491c540bp-31
			}));
			c1 = T{1}
			* polynomial(xa, std::to_array<T>({
					-0x1.a0be83b09c3d7p+0, -0x1.8bb29648c7afep+1,
					-0x1.639eb89a5975p+1,  -0x1.7b48b8cd14d9fp+0,
					-0x1.fb25a03ddc781p-2, -0x1.9cdb7dcacdfb3p-4,
					-0x1.64f7fbe544f07p-7, -0x1.9a3c3874b3919p-12
			}))
			/ polynomial(xa, std::to_array<T>({
				0x1p+0,                 0x1.55b5d06f1c2dep+0,
				0x1.b998ffae5528ep-1,   0x1.46884d56cb49bp-2,
				0x1.18e8848a2cc38p-4,   0x1.ee7f90e8d480cp-8,
				0x1.1c6a194029df4p-12, -0x1.03d1306b29028p-31
			}));
		}

		auto xb = T{1} - std::exp2(c1 * xa);
		return xb < T{1}
		? x * c0
		: std::copysign(std::isinf(xb) ? xb : 1.f, x);
	}

	// inverse real error function approximation based on "Approximating the erfinv function" by Mark Giles
	template<typename T>
	requires std::floating_point<T>
	auto erfinv(T x) noexcept -> T {
		auto w = -std::log((T{1} - x) * (T{1} + x));

		auto w1 = w - T{2.5};
		auto w2 = math::sqrt(w) - T{3};

		auto p1 = T(polynomial(w1, std::to_array<T>({
			1.50140941,     0.246640727,
			-0.00417768164, -0.00125372503,
			0.00021858087, -4.39150654e-06,
			-3.5233877e-06,  3.43273939e-07,
			2.81022636e-08
		})));

		auto p2 = T(polynomial(w2, std::to_array<T>({
			2.83297682,     1.00167406,
			0.00943887047, -0.0076224613,
			0.00573950773, -0.00367342844,
			0.00134934322,  0.000100950558,
			-0.000200214257
		})));

		return (w < T{5} ? p1 : p2) * x;
	}

	template<typename T>
	requires std::floating_point<T>
	auto gaussian(T x, T mu, T sigma) noexcept -> T {
		return std::exp(-math::sqr(x - mu) / (T{2} * math::sqr(sigma))) / (math::sqrt(T{2} * pi) * sigma);
	}

	template<typename T>
	requires std::floating_point<T>
	auto gaussian_cdf(T x, T mu, T sigma) noexcept -> T {
		return T{0.5} * (T{1} + erf((x - mu) / sigma / math::sqrt(T{2})));
	}
}
