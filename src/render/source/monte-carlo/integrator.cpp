#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::monte_carlo {
    Context::Context() noexcept {
        accel = accel::Acceleration::entity("/accel");
        emitter = emitter::Emitter::entity("/emitter");
        sampler = sampler::Sampler::entity("/sampler");
        filter = filter::Filter::entity("/filter");
        lens = photo::Lens::entity("/lens");
        film = photo::proxy::Film::entity("/film");
    }

    auto Integrator::init() noexcept -> void {
        MTT_DESERIALIZE(Radiative_Integrator, Restir_Integrator);
    }
}
