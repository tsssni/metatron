#pragma once
#include <metatron/geometry/shape/shape.hpp>

namespace metatron::shape {
	struct Mesh final: Shape {
		auto bounding_box(usize idx = 0uz) const -> math::Bounding_Box;
		auto operator()(
			math::Ray const& r,
			usize idx = 0uz
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idx = 0uz
		) const -> std::optional<Interaction>;

		auto static from_path(std::string_view path) -> std::unique_ptr<Mesh>;

	private:
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
