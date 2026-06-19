#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::monte_carlo {
    auto Integrator::init() noexcept -> void {
        MTT_DESERIALIZE(Radiative_Integrator, Restir_Integrator);
    }
}
