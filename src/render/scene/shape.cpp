#include <metatron/render/scene/serde.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/shape/sphere.hpp>

namespace mtt::scene {
    auto shape_init() noexcept -> void {
        using namespace shape;
        auto& vec = stl::vector<Shape>::instance();
        vec.emplace_type<Mesh>();
        vec.emplace_type<Sphere>();
        MTT_DESERIALIZE(Shape, Mesh, Sphere);
    }
}
