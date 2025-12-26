implementing monte_carlo;

namespace mtt::monte_carlo {
    public struct Volume_Path_Integrator: Integrator {
        public func sample(Context ctx) -> Optional<spectra::Stochastic_Spectrum> {
            return none;
        }
    }
}
