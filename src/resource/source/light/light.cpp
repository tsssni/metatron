#include <metatron/resource/light/light.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::light {
    auto Light::init() noexcept -> void {
        MTT_DESERIALIZE(
            Parallel_Light,
            Point_Light,
            Spot_Light,
            Area_Light,
            Environment_Light,
            Atomosphere_Light
        );
        Atomosphere_Light::init();
    }
}
