#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>

namespace mtt::media {
    auto Phase::to_phase(
        cref<stsp> spec
    ) const noexcept -> obj<phase::Phase_Function> {
        switch (function) {
        case Phase::Function::henyey_greenstein:
            return make_obj<
                phase::Phase_Function,
                phase::Henyey_Greenstein_Phase_Function
            >(g, spec);
        };
    }
}
