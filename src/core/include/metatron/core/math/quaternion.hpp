#pragma once
#include <metatron/core/math/vector.hpp>
#include <tuple>

namespace metatron::math {
    template<typename T>
    requires std::floating_point<T>
    struct Quaternion final {
    public:
		Quaternion() = default;
        Quaternion(T x, T y, T z, T w): data{x, y, z, w} {}
        explicit Quaternion(Vector<T, 4> const& v): data{v} {}

		auto static from_axis_angle(Vector<T, 3> const& axis, T const& angle) -> Quaternion<T> {
			auto half_angle = angle * T{0.5};
			auto sin_half = std::sin(half_angle);
			auto cos_half = std::cos(half_angle);
			return Quaternion{
				axis[0] * sin_half,
				axis[1] * sin_half,
				axis[2] * sin_half,
				cos_half
			};
		}

		auto static from_rotation_between(Vector<T, 3> const& from, Vector<T, 3> const& to) -> Quaternion<T> {
			auto axis = math::normalize(cross(from, to));
			auto rad = angle(from, to);
			return from_axis_angle(axis, rad);
		}

		auto operator[](usize idx) -> T& {
			return data[idx];
		}

		auto operator[](usize idx) const -> T const& {
			return data[idx];
		}
        
        auto operator*(Quaternion const& rhs) const -> Quaternion {
			// Hamilton
			auto [x, y, z, w] = std::make_tuple(data[0], data[1], data[2], data[3]);
			auto [rx, ry, rz, rw] = std::make_tuple(rhs.data[0], rhs.data[1], rhs.data[2], rhs.data[3]);
            return Quaternion{
                w * rx + x * rw + y * rz - z * ry,
                w * ry + y * rw + z * rx - x * rz,
                w * rz + z * rw + x * ry - y * rx,
                w * rw - x * rx - y * ry - z * rz
            };
        }

        auto operator*(T const& scalar) const -> Quaternion<T> {
            return Quaternion{data * scalar};
        }

        auto operator+(Quaternion const& rhs) const -> Quaternion<T> {
            return Quaternion{data + rhs.data};
        }

        auto operator-() const -> Quaternion<T> {
            return Quaternion{-data};
        }

		auto operator<=>(Quaternion<T> const&) const = default;

		explicit operator Vector<T, 4>() const {
			return data;
		}

		explicit operator Matrix<T, 4, 4>() const {
			auto [x, y, z, w] = std::make_tuple(data[0], data[1], data[2], data[3]);
			return Matrix<T, 4, 4>{
				{T{1.0}-T{2.0}*(y*y+z*z), T{2.0}*(x*y-z*w),        T{2.0}*(x*z+y*w),        T{0.0}},
				{T{2.0}*(x*y+z*w),        T{1.0}-T{2.0}*(x*x+z*z), T{2.0}*(y*z-x*w),        T{0.0}},
				{T{2.0}*(x*z-y*w),        T{2.0}*(y*z+x*w),        T{1.0}-T{2.0}*(x*x+y*y), T{0.0}},
				{T(0.0),                  T{0.0},                  T{0.0},                  T{1.0}}
			};
		}

    private:
        Vector<T, 4> data;
    };

    template<typename T>
    requires std::floating_point<T>
	auto constexpr slerp(Quaternion<T> const& q0, Quaternion<T> const& q1, T const& t) -> Quaternion<T> {
		// quaternion fits x^2 + y^2 + z^2 + w^2 = 1, so it's 4D sphere which could use slerp

		auto q0v = Vector<T, 4>{q0};
		auto q1v = Vector<T, 4>{q1};
		auto cos_theta = dot(q0v, q1v);
		
		// use lerp with small angle
		if (cos_theta > T{0.9995}) {
			return Quaternion{normalize(lerp(q0v, q1v, t))};
		}

		// ensure shortest path
		auto q1v_adj = q1v;
		if (cos_theta < T{0}) {
			q1v_adj = -q1v_adj;
			cos_theta = -cos_theta;
		}

		auto theta = angle(q0v, q1v_adj);
		auto sin_theta = std::sin(theta);
		
		auto scale1 = std::sin((T{1} - t) * theta) / sin_theta;
		auto scale2 = std::sin(t * theta) / sin_theta;
		
		return {normalize(scale1 * q0v + scale2 * q1v_adj)};
	}

    template<typename T>
    requires std::floating_point<T>
	auto constexpr conj(Quaternion<T> const& q) -> Quaternion<T> {
		return {-q[0], -q[1], -q[2], q[3]};
	}

    template<typename T>
    requires std::floating_point<T>
	auto constexpr rotate(Vector<T, 4> const& x, Quaternion<T> const& q) -> math::Vector<T, 4> {
		auto p = Quaternion{x[0], x[1], x[2], T{0}};
		auto r = q * p * conj(q);
		return {r[0], r[1], r[2], T{0}};
	}
}
