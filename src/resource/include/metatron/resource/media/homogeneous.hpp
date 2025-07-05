#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/phase/phase-function.hpp>

namespace mtt::media {
	struct Homogeneous_Medium final: Medium {
		Homogeneous_Medium(
			phase::Phase_Function const* phase,
			spectra::Spectrum const* sigma_a,
			spectra::Spectrum const* sigma_s,
			spectra::Spectrum const* emission
		);
		auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;

	private:
		phase::Phase_Function const* phase;
		spectra::Spectrum const* sigma_a;
		spectra::Spectrum const* sigma_s;
		spectra::Spectrum const* emission;
	};
}
