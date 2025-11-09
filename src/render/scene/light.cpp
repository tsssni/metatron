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
        light::Sunsky_Light::init();

        auto& vec = stl::vector<Light>::instance();
        vec.emplace_type<Parallel_Light>();
        vec.emplace_type<Point_Light>();
        vec.emplace_type<Spot_Light>();
        vec.emplace_type<Area_Light>();
        vec.emplace_type<Environment_Light>();
        vec.emplace_type<Sunsky_Light>();

        MTT_DESERIALIZE(Light
        , Parallel_Light
        , Point_Light
        , Spot_Light
        , Area_Light
        , Environment_Light
        , Sunsky_Light);
    }
}
