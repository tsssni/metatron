#pragma once
#include <metatron/resource/shape/shape.hpp>

namespace mtt::shape {
	struct Mesh final {
		Mesh(
			std::vector<math::Vector<usize, 3>>&& indices,
			std::vector<math::Vector<f32, 3>>&& vertices,
			std::vector<math::Vector<f32, 3>>&& normals,
			std::vector<math::Vector<f32, 2>>&& uvs
		) noexcept;

		auto size() const noexcept -> usize;
		auto bounding_box(
			math::Matrix<f32, 4, 4> const& t,
			usize idx
		) const noexcept -> math::Bounding_Box;
		auto operator()(
			math::Ray const& r,
			usize idx
		) const noexcept -> std::optional<Interaction>;
		// sphere triangle sampling: https://pbr-book.org/4ed/Shapes/Triangle_Meshes
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idx
		) const noexcept -> std::optional<Interaction>;
		auto pdf(
			math::Ray const& r,
			math::Vector<f32, 3> const& np = {},
			usize idx = 0uz
		) const noexcept -> f32;

		auto static from_path(std::string_view path) noexcept -> Mesh;

	private:
		template<typename T>
		auto blerp(
			std::vector<T> const& traits,
			math::Vector<f32, 3> const& b,
			usize idx
		) const noexcept -> T {
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
