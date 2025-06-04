#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/phase/phase-function.hpp>
#include <memory>

namespace metatron::media {
	struct Homogeneous_Medium final: Medium {
		Homogeneous_Medium(
			std::unique_ptr<spectra::Spectrum> sigma_a,
			std::unique_ptr<spectra::Spectrum> sigma_s,
			std::unique_ptr<spectra::Spectrum> L,
			f32 g
		);
		auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;

	private:
		std::unique_ptr<spectra::Spectrum> sigma_a;
		std::unique_ptr<spectra::Spectrum> sigma_s;
		std::unique_ptr<spectra::Spectrum> L;
		f32 g;
	};
}
