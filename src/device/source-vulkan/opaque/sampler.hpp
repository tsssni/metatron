#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/sampler.hpp>

namespace mtt::opaque {
    struct Sampler::Impl final {
        vk::UniqueSampler sampler;
        auto mode(Mode mode) noexcept -> vk::SamplerAddressMode;
        auto border(Border border) noexcept -> vk::BorderColor;
    };
}
