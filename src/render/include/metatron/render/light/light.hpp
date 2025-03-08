#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/render/spectra/spectrum.hpp>
#include <metatron/geometry/intr/interaction.hpp>
#include <memory>

namespace metatron::light {
	struct Light {
		struct Sample final {
			std::unique_ptr<spectra::Spectrum> L;
			math::Vector<f32, 3> wi;
			f32 pdf;
		};

		auto virtual emit(math::Ray const& r) const -> std::unique_ptr<spectra::Spectrum> = 0;
		auto virtual sample(intr::Interaction const& intr, math::Vector<f32, 2> const& u) const -> Sample = 0;
	};
}
