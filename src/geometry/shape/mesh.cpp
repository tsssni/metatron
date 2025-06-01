#include <metatron/geometry/shape/mesh.hpp>
#include <metatron/core/math/transform.hpp>

namespace metatron::shape {
	auto Mesh::bounding_box(usize idx) const -> math::Bounding_Box {
		auto prim = indices[idx];
		auto p_min = math::min(vertices[prim[0]], vertices[prim[1]], vertices[prim[2]]);
		auto p_max = math::max(vertices[prim[0]], vertices[prim[1]], vertices[prim[2]]);
		return {p_min, p_max};
	}

	auto Mesh::operator()(
		math::Ray const& r,
		usize idx
	) const -> std::optional<Interaction> {
		auto T = math::Matrix<f32, 4, 4>{math::Transform{-r.o}};
		auto P = math::Matrix<f32, 4, 4>{1.f};
		auto S = math::Matrix<f32, 4, 4>{
			{1.f, 0.f, -r.d[0] / r.d[2], 0.f,},
			{0.f, 1.f, -r.d[1] / r.d[2], 0.f,},
			{0.f, 1.f, -1.f / r.d[2], 0.f,},
			{0.f, 0.f, 0.f, 1.f,},
		};
		
		std::swap(P[2], P[math::maxi(r.d)]);
		auto prim = indices[idx];
		auto local_to_shear = S | P | T;
		auto v = math::Vector<math::Vector<f32, 4>, 3>{
			local_to_shear | math::expand(vertices[prim[0]], 1.f),
			local_to_shear | math::expand(vertices[prim[1]], 1.f),
			local_to_shear | math::expand(vertices[prim[2]], 1.f),
		};

		auto ef = [](
			math::Vector<f32, 2> p,
			math::Vector<f32, 2> v0,
			math::Vector<f32, 2> v1
		) -> f32 {
			return math::determinant(math::Matrix<f32, 2, 2>{p - v0, v1 - v0});
		};
		auto e = math::Vector<f32, 3>{
			ef({0.f}, v[1], v[2]),
			ef({0.f}, v[2], v[0]),
			ef({0.f}, v[0], v[1]),
		};
		auto det = math::sum(e);
		auto b = math::guarded_div(e, det);

		if (false
		|| std::abs(det) < math::epsilon<f32>
		|| std::signbit(e[0]) != std::signbit(e[1])
		|| std::signbit(e[1]) != std::signbit(e[2])
		) {
			return {};
		}

		auto t = math::guarded_div(math::blerp(v, e)[2], det);
		if (std::abs(t) < math::epsilon<f32>) {
			return {};
		}

		return shape::Interaction{
			blerp(vertices, b, idx),
			math::normalize(blerp(normals, b, idx)),
			blerp(uvs, b, idx),
			t,
			dpdu[idx],
			dpdv[idx],
			dndu[idx],
			dndv[idx],
			1.f
		};
	}

	auto Mesh::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u,
		usize idx
	) const -> std::optional<Interaction> {
		auto prim = indices[idx];
		auto validate_vector = [](math::Vector<f32, 3> const& v) -> bool {
			return math::dot(v, v) >= math::epsilon<f32>;
		};

		auto a = math::normalize(vertices[prim[0]] - ctx.r.o);
		auto b = math::normalize(vertices[prim[1]] - ctx.r.o);
		auto c = math::normalize(vertices[prim[2]] - ctx.r.o);
		if (false
		|| validate_vector(a)
		|| validate_vector(b)
		|| validate_vector(c)) {
			return {};
		}

		auto n_ab = math::normalize(math::cross(a, b));
		auto n_bc = math::normalize(math::cross(b, c));
		auto n_ca = math::normalize(math::cross(c, a));
		if (false
		|| validate_vector(n_ab)
		|| validate_vector(n_bc)
		|| validate_vector(n_ca)) {
			return {};
		}

		auto alpha = math::angle(n_ab, -n_ca);
		auto beta = math::angle(n_bc, -n_ab);
		auto gamma = math::angle(n_ca, -n_bc);

		auto A_pi = alpha + beta + gamma;
		auto A_1_pi = std::lerp(math::pi, A_pi, u[0]);

		auto phi = A_1_pi - alpha;
		auto cos_a = std::cos(alpha);
		auto sin_a = std::sin(alpha);
		auto cos_p = std::cos(phi);
		auto sin_p = std::sin(phi);

		auto k_1 = cos_p + cos_a;
		auto k_2 = sin_p - sin_a * dot(a, b);
		auto cos_ac1 = 1.f
			* (k_2 + (k_2 * cos_p - k_1 * sin_p) * cos_a)
			/ ((k_2 * sin_p + k_1 * cos_p) * sin_a);
		auto sin_ac1 = math::sqrt(1.f - cos_ac1 * cos_ac1);
		auto c_1 = cos_ac1 * a + sin_ac1 * math::normalize(math::gram_schmidt(c, a));

		auto cos_bc1 = math::dot(b, c_1);
		auto cos_bc2 = 1.f - u[1] * (1.f - cos_bc1);
		auto sin_bc2 = math::sqrt(1.f - cos_bc2 * cos_bc2);
		auto d = cos_bc2 * b + sin_bc2 * math::normalize(math::gram_schmidt(c_1, b));

		auto v = math::Vector<math::Vector<f32, 3>, 3>{
			vertices[prim[0]],
			vertices[prim[1]],
			vertices[prim[2]],
		};
		auto [t, b_1, b_2] = math::cramer(
			math::transpose(
				math::Matrix<f32, 3, 3>{-d, v[1] - v[0], v[2] - v[0]}
			),
			ctx.r.o - v[0]
		);
		b_1 = std::clamp(b_1, 0.f, 1.f);
		b_2 = std::clamp(b_2, 0.f, 1.f);
		if (b_1 + b_2 > 1.f) {
			b_1 /= (b_1 + b_2);
			b_2 /= (b_1 + b_2);
		}
		auto bary = math::Vector<f32, 3>{1.f - b_1 - b_2, b_1, b_2};

		return Interaction{
			ctx.r.o + t * d,
			math::normalize(blerp(normals, b, idx)),
			blerp(uvs, bary, idx),
			t,
			dpdu[idx],
			dpdv[idx],
			dndu[idx],
			dndv[idx],
			1.f,
		};
	}

	auto Mesh::from_path(std::string_view path) -> std::unique_ptr<Mesh> {
		return std::make_unique<Mesh>();
	}
}
