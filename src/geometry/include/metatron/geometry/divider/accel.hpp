#pragma once
#include <metatron/geometry/divider/divider.hpp>
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/ray.hpp>
#include <memory>

namespace metatron::divider {
	struct Interaction final {
		Divider const* divider;
		shape::Interaction intr;
	};

	struct Node final {
		Divider const* divider{nullptr};
		math::Bounding_Box bbox;
		std::unique_ptr<Node> left;
		std::unique_ptr<Node> right;
	};

	struct Acceleration {
		virtual ~Acceleration() {}
		auto virtual operator()(math::Ray const& r) -> std::optional<Interaction> = 0;
	};
}
