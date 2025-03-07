#include <vector>
#include <cstdlib>

namespace metatron::spectra {
	struct Spectrum {
		virtual ~Spectrum();
		auto virtual operator()(f32 lambda) -> f32& = 0;
		auto virtual operator()(f32 lambda) const -> f32 const& = 0;
		auto virtual operator*(Spectrum const& spectrum) const -> f32 = 0;
	};

	struct Stochastic_Spectrum final: Spectrum {
		std::vector<f32> lambda;
		std::vector<f32> pdf;
		std::vector<f32> value;

		Stochastic_Spectrum(usize n, f32 u);

		auto operator()(f32 lambda) -> f32&;
		auto operator()(f32 lambda) const -> f32 const&;
		auto operator*(Spectrum const& spectrum) const -> f32;
	};
}
