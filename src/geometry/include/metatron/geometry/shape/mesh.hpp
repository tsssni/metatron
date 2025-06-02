#pragma once
#include <metatron/geometry/shape/shape.hpp>

namespace metatron::shape {
	struct Mesh final: Shape {
		Mesh(
			std::vector<math::Vector<usize, 3>>&& indices,
			std::vector<math::Vector<f32, 3>>&& vertices,
			std::vector<math::Vector<f32, 3>>&& normals,
			std::vector<math::Vector<f32, 2>>&& uvs
		);

		auto bounding_box(usize idx = 0uz) const -> math::Bounding_Box;
		auto operator()(
			math::Ray const& r,
			math::Vector<f32, 3> const& np = {},
			usize idx = 0uz
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idx = 0uz
		) const -> std::optional<Interaction>;

		auto static from_path(std::string_view path) -> std::unique_ptr<Mesh>;

	private:
		template<typename T>
		auto blerp(
			std::vector<T> const& traits,
			math::Vector<f32, 3> const& b,
			usize idx
		) const -> T {
			if (traits.empty()) {
				return {};
			}
			auto prim = indices[idx];
			return math::blerp(
				math::Vector<T, 3>{
					traits[prim[0]],
					traits[prim[1]],
					traits[prim[2]],
				}, b
			);
		}

		std::vector<math::Vector<usize, 3>> indices;

		std::vector<math::Vector<f32, 3>> vertices;
		std::vector<math::Vector<f32, 3>> normals;
		std::vector<math::Vector<f32, 2>> uvs;

		std::vector<math::Vector<f32, 3>> dpdu;
		std::vector<math::Vector<f32, 3>> dpdv;
		std::vector<math::Vector<f32, 3>> dndu;
		std::vector<math::Vector<f32, 3>> dndv;
	};
}
