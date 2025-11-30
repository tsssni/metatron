#include <metatron/resource/spectra/constant.hpp>

namespace mtt::spectra {
    auto Constant_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        return x;
    }
}
