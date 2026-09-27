#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::photo {
    auto lens::init() noexcept -> void {
        MTT_DESERIALIZE(Pinhole_Lens, Thin_Lens);
    }
}
