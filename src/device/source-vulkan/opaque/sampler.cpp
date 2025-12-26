#include "sampler.hpp"

namespace mtt::opaque {
    auto Sampler::Impl::mode(Mode mode) noexcept -> vk::SamplerAddressMode {
        switch (mode) {
        case Mode::repeat: return vk::SamplerAddressMode::eRepeat;
        case Mode::mirror: return vk::SamplerAddressMode::eMirroredRepeat;
        case Mode::clamp: return vk::SamplerAddressMode::eClampToEdge;
        case Mode::border: return vk::SamplerAddressMode::eClampToBorder;
        }
    }

    auto Sampler::Impl::border(Border border) noexcept -> vk::BorderColor {
        switch (border) {
        case Border::transparent: return vk::BorderColor::eFloatTransparentBlack;
        case Border::opaque: return vk::BorderColor::eFloatOpaqueBlack;
        case Border::white: return vk::BorderColor::eFloatOpaqueWhite;
        }
    }

    Sampler::Sampler(cref<Descriptor> desc) noexcept
    : mode(desc.mode), border(desc.border) {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto mode = impl->mode(desc.mode);
        auto border = impl->border(desc.border);
        impl->sampler = command::guard(device.createSamplerUnique(vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = mode,
            .addressModeV = mode,
            .addressModeW = mode,
            .mipLodBias = 0.f,
            .anisotropyEnable = true,
            .maxAnisotropy = 16.f,
            .compareEnable = false,
            .compareOp = vk::CompareOp::eNever,
            .minLod = 0.f,
            .maxLod = vk::LodClampNone,
            .borderColor = border,
            .unnormalizedCoordinates = false,
        }));
    }
}
