#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::media {
    auto init() noexcept -> void {
        MTT_DESERIALIZE(Homogeneous_Medium, Heterogeneous_Medium, Vaccum_Medium);
        Medium::vs::push<Vaccum_Medium>("/medium/vaccum", {});
    }
}
