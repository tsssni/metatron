#pragma once
#include <metatron/render/light/light.hpp>
#include <metatron/geometry/material/texture/spectrum/image.hpp>
#include <metatron/core/image/image.hpp>

namespace metatron::light {
	struct Environment_Light final: Light {
		Environment_Light(std::unique_ptr<image::Image> env_map);
		auto operator()(math::Ray const& r, spectra::Stochastic_Spectrum const& Lo) const -> std::optional<spectra::Stochastic_Spectrum>;
		auto sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction>;
	private:
		auto operator()(math::Vector<f32, 2> const& wi, spectra::Stochastic_Spectrum const& Lo) const -> std::optional<spectra::Stochastic_Spectrum>;
		std::unique_ptr<image::Image> env_map;
	};
}
