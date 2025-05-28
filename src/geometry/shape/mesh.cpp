#include <metatron/geometry/shape/mesh.hpp>

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
		auto prim = indices[idx];
		auto v = math::Matrix<f32, 3, 3>{
			vertices[prim[0]],
			vertices[prim[1]],
			vertices[prim[2]],
		};
		auto e = math::Matrix<f32, 3, 3>{
			v[1] - v[0],
			v[2] - v[1],
			v[0] - v[2],
		};

		return std::nullopt;
	}

	auto Mesh::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u,
		usize idx
	) const -> std::optional<Interaction> {
		auto prim = indices[idx];
		auto vs = math::Matrix<f32, 3, 3>{
			vertices[prim[0]],
			vertices[prim[1]],
			vertices[prim[2]]
		};
		return std::nullopt;
	}

	auto Mesh::from_path(std::string_view path) -> std::unique_ptr<Mesh> {
		return std::make_unique<Mesh>();
	}
}
