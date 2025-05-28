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
		if (false
		|| std::abs(det) < math::epsilon<f32>
		|| std::signbit(e[0]) != std::signbit(e[1])
		|| std::signbit(e[1]) != std::signbit(e[2])
		) {
			return {};
		}

		auto t = math::lerp(v, e)[2] / det;
		if (std::abs(t) < math::epsilon<f32>) {
			return {};
		}

		auto b = e / det;
		auto blerp = [b, prim]<typename T>(std::vector<T> const& traits) -> T {
			if (traits.empty()) {
				return {};
			}
			return math::lerp(
				math::Vector<T, 3>{
					traits[prim[0]],
					traits[prim[1]],
					traits[prim[2]],
				}, b
			);
		};

		return shape::Interaction{
			blerp(vertices),
			math::normalize(blerp(normals)),
			blerp(uvs),
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
		return {};
	}

	auto Mesh::from_path(std::string_view path) -> std::unique_ptr<Mesh> {
		return std::make_unique<Mesh>();
	}
}
