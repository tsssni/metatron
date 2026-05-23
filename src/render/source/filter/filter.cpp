#include <metatron/render/filter/filter.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::filter {
    auto Filter::init() noexcept -> void {
        MTT_DESERIALIZE(Box_Filter, Gaussian_Filter, Lanczos_Filter);
    }
}
