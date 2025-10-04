#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/core/math/vector.hpp>
#include <variant>

namespace mtt::compo {
    struct Parallel_Light final {
        ecs::Entity spectrum;
        i32 parallel{0};
    };

    struct Point_Light final {
        ecs::Entity spectrum;
        i32 point{0};
    };

    struct Spot_Light final {
        ecs::Entity spectrum;
        f32 falloff_start_theta;
        f32 falloff_end_theta;
        i32 spot{0};
    };

    struct Environment_Light final {
        ecs::Entity env_map;
        ecs::Entity sampler{"/sampler/default"_et};
        i32 environment{0};
    };

    struct Sunsky_Light final {
        math::Vector<f32, 2> direction;
        f32 turbidity;
        f32 albedo;
        f32 aperture;
        f32 temperature;
        f32 intensity;
        i32 sunsky{0};
    };

    using Light = std::variant<
        Parallel_Light,
        Point_Light,
        Spot_Light,
        Environment_Light,
        Sunsky_Light
    >;
}
