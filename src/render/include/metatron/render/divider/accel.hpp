#pragma once
#include <metatron/render/divider/divider.hpp>
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/ray.hpp>
#include <memory>

namespace metatron::divider {
	struct Node final {
		Divider const* divider{nullptr};
		math::Bounding_Box bbox;
		std::unique_ptr<Node> left;
		std::unique_ptr<Node> right;
	};

	struct Acceleration {
		virtual ~Acceleration() {}
		auto virtual operator()(math::Ray const& r) const -> std::optional<Divider const*> = 0;
	};
}
