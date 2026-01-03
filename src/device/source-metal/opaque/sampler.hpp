#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/sampler.hpp>

namespace mtt::opaque {
    struct Sampler::Impl final {
        mtl<MTL::SamplerState> sampler;
        auto mode(Mode mode) noexcept -> MTL::SamplerAddressMode;
        auto border(Border border) noexcept -> MTL::SamplerBorderColor;
    };
}
