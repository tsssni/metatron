#pragma once

namespace mtt::render {

struct [[mtt::reflection]] Constant_Spectrum final {
    f32 x;
    auto operator()(f32 lambda) const noexcept -> f32;
};

}
