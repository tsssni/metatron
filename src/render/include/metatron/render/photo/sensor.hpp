#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <memory>

namespace metatron::photo {
	struct Sensor final {
		Sensor(
			std::unique_ptr<spectra::Spectrum> r,
			std::unique_ptr<spectra::Spectrum> g,
			std::unique_ptr<spectra::Spectrum> b
		);

		auto operator()(spectra::Stochastic_Spectrum const& spectrum) -> math::Vector<f32, 3>;

	private:
		std::unique_ptr<spectra::Spectrum> r;
		std::unique_ptr<spectra::Spectrum> g;
		std::unique_ptr<spectra::Spectrum> b;
	};
}
