#include <metatron/render/sampler/sampler.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::sampler {
    auto init() noexcept -> void {
        z_sobol::init();
        MTT_DESERIALIZE(
            Independent_Sampler,
            Halton_Sampler,
            Z_Sobol_Sampler
        );
    }
}
