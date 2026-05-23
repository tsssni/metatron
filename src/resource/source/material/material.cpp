#include <metatron/resource/material/material.hpp>
#include <metatron/resource/serde/serde.hpp>
#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/material/interface.hpp>

namespace mtt::material {
    auto Material::init() noexcept -> void {
        MTT_DESERIALIZE(
        Physical_Material,
        Interface_Material);
    }
}
