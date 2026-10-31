#pragma once
#include <cmath>

namespace mtt::stl {

using std::abs;
using std::isnan;
using std::isinf;
using std::isfinite;
using std::fmod;
using std::sqrt;
using std::cbrt;
using std::pow;
using std::exp;
using std::log;
using std::log2;
using std::sin;
using std::cos;
using std::cosh;
using std::asin;
using std::acos;
using std::atan2;
using std::atanh;
using std::floor;
using std::ceil;
using std::round;
using std::erf;
using std::fma;
using std::signbit;

auto inline abs(f16 x) noexcept -> f16 { return __builtin_fabsf16(x); }
auto inline constexpr isnan(f16 x) noexcept -> bool { return __builtin_isnan(x); }
auto inline constexpr isinf(f16 x) noexcept -> bool { return __builtin_isinf(x); }
auto inline constexpr isfinite(f16 x) noexcept -> bool { return __builtin_isfinite(x); }
auto inline constexpr signbit(f16 x) noexcept -> bool { return __builtin_signbit(x); }

auto inline fmod(f16 x, f16 y) noexcept -> f16 { return __builtin_elementwise_fmod(x, y); }
auto inline sqrt(f16 x) noexcept -> f16 { return __builtin_elementwise_sqrt(x); }
auto inline cbrt(f16 x) noexcept -> f16 { return (f16)stl::cbrt((f32)x); }
auto inline pow(f16 x, f16 y) noexcept -> f16 { return __builtin_elementwise_pow(x, y); }
auto inline exp(f16 x) noexcept -> f16 { return __builtin_elementwise_exp(x); }
auto inline log(f16 x) noexcept -> f16 { return __builtin_elementwise_log(x); }
auto inline log2(f16 x) noexcept -> f16 { return __builtin_elementwise_log2(x); }
auto inline sin(f16 x) noexcept -> f16 { return __builtin_elementwise_sin(x); }
auto inline cos(f16 x) noexcept -> f16 { return __builtin_elementwise_cos(x); }
auto inline cosh(f16 x) noexcept -> f16 { return __builtin_elementwise_cosh(x); }
auto inline asin(f16 x) noexcept -> f16 { return __builtin_elementwise_asin(x); }
auto inline acos(f16 x) noexcept -> f16 { return __builtin_elementwise_acos(x); }
auto inline atan2(f16 y, f16 x) noexcept -> f16 { return __builtin_elementwise_atan2(y, x); }
auto inline atanh(f16 x) noexcept -> f16 { return (f16)stl::atanh((f32)x); }
auto inline floor(f16 x) noexcept -> f16 { return __builtin_elementwise_floor(x); }
auto inline ceil(f16 x) noexcept -> f16 { return __builtin_elementwise_ceil(x); }
auto inline round(f16 x) noexcept -> f16 { return __builtin_elementwise_round(x); }
auto inline erf(f16 x) noexcept -> f16 { return (f16)stl::erf((f32)x); }
auto inline fma(f16 x, f16 y, f16 z) noexcept -> f16 { return __builtin_elementwise_fma(x, y, z); }

}
