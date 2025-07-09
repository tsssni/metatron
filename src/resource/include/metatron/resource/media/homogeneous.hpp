#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/phase/phase-function.hpp>

namespace mtt::media {
	struct Homogeneous_Medium final: Medium {
		Homogeneous_Medium(
			phase::Phase_Function const* phase,
			view<spectra::Spectrum> sigma_a,
			view<spectra::Spectrum> sigma_s,
			view<spectra::Spectrum> emission
		);
		auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;

	private:
		phase::Phase_Function const* phase;
		view<spectra::Spectrum> sigma_a;
		view<spectra::Spectrum> sigma_s;
		view<spectra::Spectrum> emission;
	};
}
