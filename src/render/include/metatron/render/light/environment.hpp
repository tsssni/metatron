#pragma once
#include <metatron/render/light/light.hpp>
#include <metatron/render/material/texture/spectrum.hpp>

namespace metatron::light {
	struct Environment_Light final: Light {
		Environment_Light(std::unique_ptr<material::Spectrum_Image_Texture> texture);
		auto operator()(math::Ray const& r) const -> std::optional<Spectrum>;
		auto sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction>;
	private:
		std::unique_ptr<material::Spectrum_Image_Texture> texture;
	};
}
