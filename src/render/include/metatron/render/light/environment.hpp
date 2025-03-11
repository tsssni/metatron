#pragma once
#include <metatron/render/light/light.hpp>
#include <metatron/render/material/spectrum.hpp>

namespace metatron::light {
	struct Environment_Light final: Light {
		Environment_Light(std::unique_ptr<material::Spectrum_Image_Texture> texture);
		auto virtual emit(math::Ray const& r) const -> std::unique_ptr<spectra::Spectrum>;
		auto virtual sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> Interaction;
	private:
		std::unique_ptr<material::Spectrum_Image_Texture> texture;
	};
}
