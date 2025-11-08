#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/resource/light/parallel.hpp>
#include <metatron/resource/light/point.hpp>
#include <metatron/resource/light/spot.hpp>
#include <metatron/resource/light/area.hpp>
#include <metatron/resource/light/environment.hpp>
#include <metatron/resource/light/sunsky.hpp>

namespace mtt::scene {
    auto light_init() noexcept -> void {
        auto& vec = stl::vector<light::Light>::instance();
        vec.emplace_type<light::Parallel_Light>();
        vec.emplace_type<light::Point_Light>();
        vec.emplace_type<light::Spot_Light>();
        vec.emplace_type<light::Area_Light>();
        vec.emplace_type<light::Environment_Light>();
        vec.emplace_type<light::Sunsky_Light>();

        light::Sunsky_Light::init();
    }
}
