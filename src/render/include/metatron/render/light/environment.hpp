#pragma once
#include <metatron/render/light/light.hpp>
#include <metatron/core/image/image.hpp>

namespace metatron::light {
	struct Environment_Light final: Light {
		Environment_Light(std::unique_ptr<image::Image> env_map);
		auto operator()(math::Ray const& r) const -> std::optional<Spectrum>;
		auto sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction>;
	private:
		std::unique_ptr<image::Image> env_map;
	};
}
