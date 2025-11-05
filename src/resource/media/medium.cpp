#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>

namespace mtt::media {
    auto Phase::to_phase(
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> poly<phase::Phase_Function> {
        switch (function) {
        case Phase::Function::henyey_greenstein:
            return make_poly<
                phase::Phase_Function,
                phase::Henyey_Greenstein_Phase_Function
            >(g, spec);
        };
    }
}
