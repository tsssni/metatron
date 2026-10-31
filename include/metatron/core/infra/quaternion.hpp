#pragma once
#include <metatron/core/stl/cmath.hpp>
#include <metatron/core/stl/concept.hpp>
#include <metatron/core/infra/vector.hpp>

namespace mtt::math {

template<stl::floating_point T>
struct quaternion final {
    quaternion() = default;
    quaternion(T x, T y, T z, T w);
    explicit quaternion(vector<T, 4> const& v);

    auto constexpr static from_axis_angle(vector<T, 3> const& axis, T const& angle) noexcept -> quaternion<T>;
    auto constexpr static from_rotation_between(vector<T, 3> const& from, vector<T, 3> const& to) noexcept -> quaternion<T>;

    auto constexpr operator[](usize idx) noexcept -> T&;
    auto constexpr operator[](usize idx) const noexcept -> T const&;

    auto constexpr operator*(quaternion const& rhs) const noexcept -> quaternion<T>;
    auto constexpr operator*(T const& scalar) const noexcept -> quaternion<T>;
    auto constexpr operator+(quaternion const& rhs) const noexcept -> quaternion<T>;
    auto constexpr operator-() const noexcept -> quaternion<T>;
    auto constexpr operator<=>(quaternion<T> const&) const = default;

    explicit constexpr operator vector<T, 4>() const;
    explicit constexpr operator matrix<T, 4, 4>() const;

private:
    vector<T, 4> data;
};

template<stl::floating_point T>
quaternion<T>::quaternion(T x, T y, T z, T w): data{x, y, z, w} {}
template<stl::floating_point T>
quaternion<T>::quaternion(vector<T, 4> const& v): data{v} {}

template<stl::floating_point T>
auto constexpr quaternion<T>::from_axis_angle(vector<T, 3> const& axis, T const& angle) noexcept -> quaternion<T> {
    auto half_angle = angle * T(0.5);
    auto sin_half = stl::sin(half_angle);
    auto cos_half = stl::cos(half_angle);
    return {
        axis[0] * sin_half,
        axis[1] * sin_half,
        axis[2] * sin_half,
        cos_half,
    };
}

template<stl::floating_point T>
auto constexpr quaternion<T>::from_rotation_between(vector<T, 3> const& from, vector<T, 3> const& to) noexcept -> quaternion<T> {
    auto axis = cross(from, to);
    if (length(axis) < epsilon<f32>) {
        auto perp = abs(dot(from, vector<T, 3>{0, 1, 0})) >= 1 - 1e-6f
        ? vector<T, 3>{1, 0, 0}
        : vector<T, 3>{0, 1, 0};
        axis = cross(from, perp);
    }
    auto rad = angle(from, to);
    return from_axis_angle(normalize(axis), rad);
}

template<stl::floating_point T>
auto constexpr quaternion<T>::operator[](usize idx) noexcept -> T& { return data[idx]; }

template<stl::floating_point T>
auto constexpr quaternion<T>::operator[](usize idx) const noexcept -> T const& { return data[idx]; }

template<stl::floating_point T>
auto constexpr quaternion<T>::operator*(quaternion const& rhs) const noexcept -> quaternion<T> {
    // Hamilton
    auto [x, y, z, w] = data;
    auto [rx, ry, rz, rw] = rhs.data;
    return {
        w * rx + x * rw + y * rz - z * ry,
        w * ry + y * rw + z * rx - x * rz,
        w * rz + z * rw + x * ry - y * rx,
        w * rw - x * rx - y * ry - z * rz,
    };
}

template<stl::floating_point T>
auto constexpr quaternion<T>::operator*(T const& scalar) const noexcept -> quaternion<T> { return quaternion{data * scalar}; }
template<stl::floating_point T>
auto constexpr quaternion<T>::operator+(quaternion const& rhs) const noexcept -> quaternion<T> { return quaternion{data + rhs.data}; }
template<stl::floating_point T>
auto constexpr quaternion<T>::operator-() const noexcept -> quaternion<T> { return quaternion{-data}; }

template<stl::floating_point T>
constexpr quaternion<T>::operator vector<T, 4>() const { return data; }
template<stl::floating_point T>
constexpr quaternion<T>::operator matrix<T, 4, 4>() const {
    auto [x, y, z, w] = data;
    return matrix<T, 4, 4>{
        {T(1.0)-T(2.0)*(y*y+z*z), T(2.0)*(x*y-z*w),        T(2.0)*(x*z+y*w),        T(0.0)},
        {T(2.0)*(x*y+z*w),        T(1.0)-T(2.0)*(x*x+z*z), T(2.0)*(y*z-x*w),        T(0.0)},
        {T(2.0)*(x*z-y*w),        T(2.0)*(y*z+x*w),        T(1.0)-T(2.0)*(x*x+y*y), T(0.0)},
        {T(0.0),                  T(0.0),                  T(0.0),                  T(1.0)},
    };
}

template<stl::floating_point T>
auto constexpr slerp(quaternion<T> const& q0, quaternion<T> const& q1, T const& t) noexcept -> quaternion<T> {
    // quaternion fits x^2 + y^2 + z^2 + w^2 = 1, so it's 4D sphere which could use slerp
    auto q0v = vector<T, 4>{q0};
    auto q1v = vector<T, 4>{q1};
    auto cos_theta = dot(q0v, q1v);

    // use lerp with small angle
    if (cos_theta > T(0.9995))
        return quaternion{normalize(lerp(q0v, q1v, t))};

    // ensure shortest path
    auto q1v_adj = q1v;
    if (cos_theta < T(0)) {
        q1v_adj = -q1v_adj;
        cos_theta = -cos_theta;
    }
    auto theta = angle(q0v, q1v_adj);
    auto sin_theta = stl::sin(theta);
    auto scale1 = stl::sin((T(1) - t) * theta) / sin_theta;
    auto scale2 = stl::sin(t * theta) / sin_theta;

    return quaternion{normalize(scale1 * q0v + scale2 * q1v_adj)};
}

template<stl::floating_point T>
auto constexpr conj(quaternion<T> const& q) noexcept -> quaternion<T> { return {-q[0], -q[1], -q[2], q[3]}; }

template<stl::floating_point T>
auto constexpr rotate(vector<T, 4> const& x, quaternion<T> const& q) noexcept -> vector<T, 4> {
    auto p = quaternion{x[0], x[1], x[2], T(0)};
    auto r = q * p * conj(q);
    return {r[0], r[1], r[2], T(0)};
}

}

namespace mtt {

#define MTT_QUATERNION_ALIAS(p, T)\
using p##q = math::quaternion<T>;

MTT_QUATERNION_ALIAS(h, f16)
MTT_QUATERNION_ALIAS(f, f32)
MTT_QUATERNION_ALIAS(d, f64)

}
