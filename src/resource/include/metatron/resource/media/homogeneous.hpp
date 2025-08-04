#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/phase/phase-function.hpp>

namespace mtt::media {
	struct Homogeneous_Medium final {
		Homogeneous_Medium(
			poly<phase::Phase_Function> phase,
			view<spectra::Spectrum> sigma_a,
			view<spectra::Spectrum> sigma_s,
			view<spectra::Spectrum> emission
		) noexcept;
		auto sample(
			eval::Context const& ctx, f32 t_max, f32 u
		) const noexcept -> std::optional<Interaction>;

	private:
		poly<phase::Phase_Function> phase;
		view<spectra::Spectrum> sigma_a;
		view<spectra::Spectrum> sigma_s;
		view<spectra::Spectrum> sigma_e;
	};
}
