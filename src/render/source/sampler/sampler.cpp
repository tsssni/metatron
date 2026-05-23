#include <metatron/render/sampler/sampler.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::sampler {
    auto Sampler::init() noexcept -> void {
        Sobol_Sampler::init();
        MTT_DESERIALIZE(Independent_Sampler, Halton_Sampler, Sobol_Sampler);
    }
}
