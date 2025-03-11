#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/spectra/spectrum.hpp>
#include <memory>

namespace metatron::photo {
	struct Sensor final {
		Sensor(
			std::unique_ptr<spectra::Spectrum> r,
			std::unique_ptr<spectra::Spectrum> g,
			std::unique_ptr<spectra::Spectrum> b
		);

		auto operator()(spectra::Spectrum const& spectrum) -> math::Vector<f32, 3>;

	private:
		std::unique_ptr<spectra::Spectrum> r;
		std::unique_ptr<spectra::Spectrum> g;
		std::unique_ptr<spectra::Spectrum> b;
	};
}
