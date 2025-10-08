#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <variant>

namespace mtt::compo {
    struct Henyey_Greenstein_Phase_Function final {
        f32 g;
        i32 henyey_greenstein{0};
    };

    using Phase_Function = std::variant<
        Henyey_Greenstein_Phase_Function
    >;

    struct Vaccum_Medium final {
        i32 vaccum{0};
    };
    
    struct Homogeneous_Medium final {
        Phase_Function phase;
        ecs::Entity sigma_a;
        ecs::Entity sigma_s;
        ecs::Entity sigma_e;
        i32 homogeneous{0};
    };

    struct Grid_Medium final {
        std::string path;
        math::Vector<usize, 3> dimensions;
        Phase_Function phase;
        ecs::Entity sigma_a;
        ecs::Entity sigma_s;
        ecs::Entity sigma_e;
        f32 density_scale;
        i32 grid{0};
    };

    using Medium = std::variant<
        Vaccum_Medium,
        Homogeneous_Medium,
        Grid_Medium
    >;

    struct Medium_Instance final {
        ecs::Entity path;
    };
}
