implementing media;

namespace mtt::media {
    public struct Heterogeneous_Medium: Medium {
        Phase phase;
        spectra::Spectrum_Tag sigma_a;
        spectra::Spectrum_Tag sigma_s;
        spectra::Spectrum_Tag sigma_e;
        u32 majority;
        u32 density;
        f32 density_scale;

        public func sample(math::Context ctx, f32 t_max, f32 u) -> Optional<Interaction> {
            // TODO: upload nanovdb volumes
            return {};
        }
    }
}
