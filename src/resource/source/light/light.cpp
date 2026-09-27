#include <metatron/resource/light/light.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::light {
    auto init() noexcept -> void {
        MTT_DESERIALIZE(
            Area_Light,
            Environment_Light,
            Atmosphere_Light
        );
        atmosphere::init();
    }
}
