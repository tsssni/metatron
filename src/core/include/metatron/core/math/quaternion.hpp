#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::math {
    template<typename T>
    requires std::floating_point<T>
    struct Quaternion final {
        Quaternion() = default;
        Quaternion(T x, T y, T z, T w): data{x, y, z, w} {}
        explicit Quaternion(cref<Vector<T, 4>> v): data{v} {}

        auto constexpr static from_axis_angle(cref<Vector<T, 3>> axis, cref<T> angle) noexcept -> Quaternion<T> {
            auto half_angle = angle * T(0.5);
            auto sin_half = std::sin(half_angle);
            auto cos_half = std::cos(half_angle);
            return Quaternion{
                axis[0] * sin_half,
                axis[1] * sin_half,
                axis[2] * sin_half,
                cos_half,
            };
        }

        auto constexpr static from_rotation_between(cref<Vector<T, 3>> from, cref<Vector<T, 3>> to) noexcept -> Quaternion<T> {
            auto axis = cross(from, to);
            if (length(axis) < epsilon<f32>) {
                auto perp = abs(dot(from, Vector<T, 3>{0, 1, 0})) >= 1 - 1e-6f
                ? Vector<T, 3>{1, 0, 0}
                : Vector<T, 3>{0, 1, 0};
                axis = cross(from, perp);
            }
            auto rad = angle(from, to);
            return from_axis_angle(normalize(axis), rad);
        }

        auto constexpr operator[](usize idx) noexcept -> ref<T> {
            return data[idx];
        }

        auto constexpr operator[](usize idx) const noexcept -> cref<T> {
            return data[idx];
        }

        auto constexpr operator*(cref<Quaternion> rhs) const noexcept -> Quaternion<T> {
            // Hamilton
            auto [x, y, z, w] = data;
            auto [rx, ry, rz, rw] = rhs.data;
            return Quaternion{
                w * rx + x * rw + y * rz - z * ry,
                w * ry + y * rw + z * rx - x * rz,
                w * rz + z * rw + x * ry - y * rx,
                w * rw - x * rx - y * ry - z * rz,
            };
        }

        auto constexpr operator*(cref<T> scalar) const noexcept -> Quaternion<T> {
            return Quaternion{data * scalar};
        }

        auto constexpr operator+(cref<Quaternion> rhs) const noexcept -> Quaternion<T> {
            return Quaternion{data + rhs.data};
        }

        auto constexpr operator-() const noexcept -> Quaternion<T> {
            return Quaternion{-data};
        }

        auto constexpr operator<=>(cref<Quaternion<T>>) const = default;

        explicit constexpr operator Vector<T, 4>() const {
            return data;
        }

        explicit constexpr operator Matrix<T, 4, 4>() const {
            auto [x, y, z, w] = data;
            return Matrix<T, 4, 4>{
                {T(1.0)-T(2.0)*(y*y+z*z), T(2.0)*(x*y-z*w),        T(2.0)*(x*z+y*w),        T(0.0)},
                {T(2.0)*(x*y+z*w),        T(1.0)-T(2.0)*(x*x+z*z), T(2.0)*(y*z-x*w),        T(0.0)},
                {T(2.0)*(x*z-y*w),        T(2.0)*(y*z+x*w),        T(1.0)-T(2.0)*(x*x+y*y), T(0.0)},
                {T(0.0),                  T(0.0),                  T(0.0),                  T(1.0)},
            };
        }

    private:
        Vector<T, 4> data;
    };

    template<typename T>
    requires std::floating_point<T>
    auto constexpr slerp(cref<Quaternion<T>> q0, cref<Quaternion<T>> q1, cref<T> t) noexcept -> Quaternion<T> {
        // quaternion fits x^2 + y^2 + z^2 + w^2 = 1, so it's 4D sphere which could use slerp

        auto q0v = Vector<T, 4>{q0};
        auto q1v = Vector<T, 4>{q1};
        auto cos_theta = dot(q0v, q1v);

        // use lerp with small angle
        if (cos_theta > T(0.9995))
            return Quaternion{normalize(lerp(q0v, q1v, t))};

        // ensure shortest path
        auto q1v_adj = q1v;
        if (cos_theta < T(0)) {
            q1v_adj = -q1v_adj;
            cos_theta = -cos_theta;
        }

        auto theta = angle(q0v, q1v_adj);
        auto sin_theta = std::sin(theta);

        auto scale1 = std::sin((T(1) - t) * theta) / sin_theta;
        auto scale2 = std::sin(t * theta) / sin_theta;

        return {normalize(scale1 * q0v + scale2 * q1v_adj)};
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr conj(cref<Quaternion<T>> q) noexcept -> Quaternion<T> {
        return {-q[0], -q[1], -q[2], q[3]};
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr rotate(cref<Vector<T, 4>> x, cref<Quaternion<T>> q) noexcept -> Vector<T, 4> {
        auto p = Quaternion{x[0], x[1], x[2], T(0)};
        auto r = q * p * conj(q);
        return {r[0], r[1], r[2], T(0)};
    }
}

namespace mtt {
    #define MTT_QUATERNION_ALIAS(p, T)\
    using p##q = math::Quaternion<T>;

    MTT_QUATERNION_ALIAS(f, f32)
    MTT_QUATERNION_ALIAS(d, f64)
}
