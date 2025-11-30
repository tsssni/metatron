#include <metatron/render/scene/serde.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/shape/sphere.hpp>

namespace mtt::scene {
    auto shape_init() noexcept -> void {
        using namespace shape;
        MTT_DESERIALIZE(Shape, Mesh, Sphere);
    }
}
