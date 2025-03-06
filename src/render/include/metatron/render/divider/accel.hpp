#pragma once
#include <metatron/render/divider/divider.hpp>
#include <metatron/geometry/intr/interaction.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/ray.hpp>
#include <memory>

namespace metatron::divider {
	struct Intersection final {
		Divider const* divider;
		intr::Interaction intr;
	};

	struct Node final {
		Divider const* divider{nullptr};
		math::Bounding_Box bbox;
		std::unique_ptr<Node> left;
		std::unique_ptr<Node> right;
	};

	struct Acceleration {
		auto virtual intersect(math::Ray const& r) -> std::optional<Intersection> = 0;
	};
}
