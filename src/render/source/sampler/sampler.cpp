#include <metatron/render/sampler/sampler.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::sampler {
    auto Sampler::init() noexcept -> void {
        Z_Sobol_Sampler::init();
        Heitz_Sampler::init();
        MTT_DESERIALIZE(
            Independent_Sampler,
            Halton_Sampler,
            Z_Sobol_Sampler,
            Heitz_Sampler
        );
    }
}
