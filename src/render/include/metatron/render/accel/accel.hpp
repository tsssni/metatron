#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/geometry/material/material.hpp>
#include <metatron/volume/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>
#include <memory>

namespace metatron::accel {
	struct Divider final {
		shape::Shape const* shape{};
		media::Medium const* interior_medium{};
		media::Medium const* exterior_medium{};
		material::Material const* material{};
		light::Light const* light{};
		math::Transform const* local_to_world{};
		math::Transform const* interior_transform{};
		math::Transform const* exterior_transform{};
		usize primitive{0uz};
	};

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
