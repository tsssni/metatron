#include <metatron/resource/shape/mesh.hpp>
#include <metatron/core/math/transform.hpp>
#include <metatron/core/math/distribution/bilinear.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::shape {
	Mesh::Mesh(
		std::vector<math::Vector<usize, 3>>&& indices,
		std::vector<math::Vector<f32, 3>>&& vertices,
		std::vector<math::Vector<f32, 3>>&& normals,
		std::vector<math::Vector<f32, 2>>&& uvs
	):
	indices{std::move(indices)},
	vertices{std::move(vertices)},
	normals{std::move(normals)},
	uvs{std::move(uvs)} {
		dpdu.resize(this->indices.size());
		dpdv.resize(this->indices.size());
		dndu.resize(this->indices.size());
		dndv.resize(this->indices.size());

		for (auto i = 0uz; i < indices.size(); i++) {
			auto prim = this->indices[i];
			auto v = math::Vector<math::Vector<f32, 3>, 3>{
				this->vertices[prim[0]],
				this->vertices[prim[1]],
				this->vertices[prim[2]],
			};
			auto n = math::Vector<math::Vector<f32, 3>, 3>{
				this->normals[prim[0]],
				this->normals[prim[1]],
				this->normals[prim[2]],
			};
			auto uv = math::Vector<math::Vector<f32, 2>, 3>{
				this->uvs[prim[0]],
				this->uvs[prim[1]],
				this->uvs[prim[2]],
			};

			auto quit = []{
				std::printf("mesh: degenerate\n");
				std::abort();
			};
			auto A = math::transpose(math::Matrix<f32, 2, 2>{uv[0] - uv[2], uv[1] - uv[2]});
			METATRON_OPT_OR_CALLBACK(dpduv, math::cramer(A,
				math::Matrix<f32, 2, 3>{v[0] - v[2], v[1] - v[2]}
			), { quit(); });
			METATRON_OPT_OR_CALLBACK(dnduv, math::cramer(A,
				math::Matrix<f32, 2, 3>{n[0] - n[2], n[1] - n[2]}
			), { quit(); });
			dpdu[i] = dpduv[0];
			dpdv[i] = dpduv[1];
			dndu[i] = dnduv[0];
			dndv[i] = dnduv[1];
		}
	}

	auto Mesh::size() const -> usize {
		return indices.size();
	}

	auto Mesh::bounding_box(
		math::Transform const* t,
		usize idx
	) const -> math::Bounding_Box {
		auto prim = indices[idx];
		auto v = math::Vector<math::Vector<f32, 4>, 3>{
			*t | math::expand(vertices[prim[0]], 1.f),
			*t | math::expand(vertices[prim[1]], 1.f),
			*t | math::expand(vertices[prim[2]], 1.f)
		};
		auto p_min = math::min(v[0], v[1], v[2]);
		auto p_max = math::max(v[0], v[1], v[2]);
		return {p_min, p_max};
	}

	auto Mesh::operator()(
		math::Ray const& r,
		math::Vector<f32, 3> const& np,
		usize idx
	) const -> std::optional<Interaction> {
		auto T = math::Matrix<f32, 4, 4>{math::Transform{-r.o}};
		auto P = math::Matrix<f32, 4, 4>{1.f};
		
		std::swap(P[2], P[math::maxi(math::abs(r.d))]);
		auto d = P | math::expand(r.d, 0.f);
		auto S = math::Matrix<f32, 4, 4>{
			{1.f, 0.f, -d[0] / d[2], 0.f,},
			{0.f, 1.f, -d[1] / d[2], 0.f,},
			{0.f, 0.f, 1.f / d[2], 0.f,},
			{0.f, 0.f, 0.f, 1.f,},
		};

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
		auto bary = math::guarded_div(e, det);

		if (false
		|| std::abs(det) < math::epsilon<f32>
		|| std::signbit(e[0]) != std::signbit(e[1])
		|| std::signbit(e[1]) != std::signbit(e[2])
		) {
			return {};
		}

		auto p = blerp(vertices, bary, idx);
		auto n = math::normalize(blerp(normals, bary, idx));
		auto uv = blerp(uvs, bary, idx);
		auto t = math::guarded_div(math::blerp(v, e)[2], det);
		if (t < math::epsilon<f32>) {
			return {};
		}

		auto a = math::normalize(vertices[prim[0]] - r.o);
		auto b = math::normalize(vertices[prim[1]] - r.o);
		auto c = math::normalize(vertices[prim[2]] - r.o);

		auto n_ab = math::normalize(math::cross(b, a));
		auto n_bc = math::normalize(math::cross(c, b));
		auto n_ca = math::normalize(math::cross(a, c));

		auto alpha = math::angle(n_ab, -n_ca);
		auto beta = math::angle(n_bc, -n_ab);
		auto gamma = math::angle(n_ca, -n_bc);

		auto c_2 = r.d;
		auto c_1 = math::normalize(math::cross(math::cross(b, c_2), math::cross(c, a)));
		if (math::dot(c_1, a + c) < 0.f) {
			c_1 *= -1.f;
		}

		auto u = math::Vector<f32, 2>{};
		auto pdf = 1.f;
		u[1] = math::guarded_div(1.f - math::dot(b, c_2), (1.f - math::dot(b, c_1)));
		pdf = math::guarded_div(pdf, 1.f - math::dot(b, c_1));
		if (math::dot(a, c_1) > 0.99999847691f) {/* 0.1 degrees */
			u[0] = 0.f;
		} else {
			auto n_bc1 = math::normalize(math::cross(c_1, b));
			auto n_c1a = math::normalize(math::cross(a, c_1));
			auto A = alpha + beta + gamma - math::pi;
			pdf = math::guarded_div(pdf, A);

			if (math::length(n_bc1) < math::epsilon<f32> || math::length(n_c1a) < math::epsilon<f32>) {
				u = {0.5f};
			} else {
				auto A_1 = alpha + math::angle(n_bc1, -n_ab) + math::angle(n_c1a, -n_bc1) - math::pi;
				u[1] = math::guarded_div(A_1, A);
			}
		}

		if (np != math::Vector<f32, 3>{0.f}) {
			auto distr = math::Bilinear_Distribution{
				math::dot(np, b),
				math::dot(np, a),
				math::dot(np, b),
				math::dot(np, c),
			};
			pdf *= distr.pdf(u);
		}

		return shape::Interaction{
			p,
			n,
			uv,
			t,
			dpdu[idx],
			dpdv[idx],
			dndu[idx],
			dndv[idx],
			pdf,
		};
	}

	auto Mesh::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u_0,
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

		auto n_ab = math::normalize(math::cross(b, a));
		auto n_bc = math::normalize(math::cross(c, b));
		auto n_ca = math::normalize(math::cross(a, c));
		if (false
		|| validate_vector(n_ab)
		|| validate_vector(n_bc)
		|| validate_vector(n_ca)) {
			return {};
		}

		auto alpha = math::angle(n_ab, -n_ca);
		auto beta = math::angle(n_bc, -n_ab);
		auto gamma = math::angle(n_ca, -n_bc);

		auto pdf = 1.f;
		auto u = u_0;
		if (ctx.n != math::Vector<f32, 3>{0.f}) {
			auto distr = math::Bilinear_Distribution{
				math::dot(ctx.n, b),
				math::dot(ctx.n, a),
				math::dot(ctx.n, b),
				math::dot(ctx.n, c),
			};
			u = distr.sample(u_0);
			pdf *= distr.pdf(u);
		}

		auto A_pi = alpha + beta + gamma;
		auto A_1_pi = std::lerp(math::pi, A_pi, u[0]);

		auto phi = A_1_pi - alpha;
		auto cos_a = std::cos(alpha);
		auto sin_a = std::sin(alpha);
		auto cos_p = std::cos(phi);
		auto sin_p = std::sin(phi);

		auto k_1 = cos_p + cos_a;
		auto k_2 = sin_p - sin_a * dot(a, b);
		auto cos_ac1 = math::guarded_div(
			k_2 + (k_2 * cos_p - k_1 * sin_p) * cos_a,
			(k_2 * sin_p + k_1 * cos_p) * sin_a
		);
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
		METATRON_OPT_OR_RETURN(cramer, math::cramer(
			math::transpose(math::Matrix<f32, 3, 3>{-d, v[1] - v[0], v[2] - v[0]}),
			ctx.r.o - v[0]
		), {});
		auto [t, b_1, b_2] = cramer;
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
			math::guarded_div(
				math::guarded_div(pdf, (A_pi - math::pi)),
				(1.f - cos_bc1)
			),
		};
	}
}
