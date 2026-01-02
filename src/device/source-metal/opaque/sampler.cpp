#include "sampler.hpp"

namespace mtt::opaque {
    auto Sampler::Impl::mode(Mode mode) noexcept -> MTL::SamplerAddressMode {
        switch (mode) {
        case Mode::repeat: return MTL::SamplerAddressModeRepeat;
        case Mode::mirror: return MTL::SamplerAddressModeMirrorRepeat;
        case Mode::clamp: return MTL::SamplerAddressModeClampToEdge;
        case Mode::border: return MTL::SamplerAddressModeClampToBorderColor;
        }
    }

    auto Sampler::Impl::border(Border border) noexcept -> MTL::SamplerBorderColor {
        switch (border) {
        case Border::transparent: return MTL::SamplerBorderColorTransparentBlack;
        case Border::opaque: return MTL::SamplerBorderColorOpaqueBlack;
        case Border::white: return MTL::SamplerBorderColorOpaqueWhite;
        }
    }

    Sampler::Sampler(cref<Descriptor> desc) noexcept {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto mode = impl->mode(desc.mode);
        auto border = impl->border(desc.border);
        auto sesc = MTL::SamplerDescriptor::alloc()->init();
        sesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
        sesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
        sesc->setMipFilter(MTL::SamplerMipFilterLinear);
        sesc->setRAddressMode(mode);
        sesc->setSAddressMode(mode);
        sesc->setTAddressMode(mode);
        sesc->setLodBias(0.f);
        sesc->setMaxAnisotropy(16.f);
        sesc->setCompareFunction(MTL::CompareFunctionNever);
        sesc->setLodMinClamp(0.f);
        sesc->setLodMaxClamp(1024.f);
        sesc->setLodAverage(false);
        sesc->setBorderColor(border);
        sesc->setNormalizedCoordinates(true);
        sesc->setSupportArgumentBuffers(true);
        sesc->setReductionMode(MTL::SamplerReductionModeWeightedAverage);
        impl->sampler = device->newSamplerState(sesc);
    }
}
