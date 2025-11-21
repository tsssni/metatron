#include <metatron/render/scene/serde.hpp>
#include <metatron/resource/light/parallel.hpp>
#include <metatron/resource/light/point.hpp>
#include <metatron/resource/light/spot.hpp>
#include <metatron/resource/light/area.hpp>
#include <metatron/resource/light/environment.hpp>
#include <metatron/resource/light/sunsky.hpp>

namespace mtt::scene {
    auto light_init() noexcept -> void {
        using namespace light;
        MTT_DESERIALIZE(Light
        , Parallel_Light
        , Point_Light
        , Spot_Light
        , Area_Light
        , Environment_Light
        , Sunsky_Light);
        light::Sunsky_Light::init();
    }
}
