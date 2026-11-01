#include <metatron/render/spectra/constant.hpp>

namespace mtt::render {

auto Constant_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
    return x;
}

}
