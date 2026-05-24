#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::shape {
    auto Shape::init() noexcept -> void {
        MTT_DESERIALIZE(Mesh, Sphere);
    }
}
