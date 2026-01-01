#include "image.hpp"

namespace mtt::opaque {
    auto Image::Impl::format(cref<muldim::Image> image) noexcept -> MTL::PixelFormat {
        auto channels = image.channels;
        auto stride = image.stride;
        auto linear = image.linear;

        if (stride == 1) {
            if (channels == 1) {
                if (linear) return MTL::PixelFormatR8Unorm;
                else return MTL::PixelFormatR8Unorm_sRGB;
            } else if (channels == 2) {
                if (linear) return MTL::PixelFormatRG8Unorm;
                else return MTL::PixelFormatRG8Unorm_sRGB;
            } else if (channels == 4) {
                if (linear) return MTL::PixelFormatRGBA8Unorm;
                else return MTL::PixelFormatRGBA8Unorm_sRGB;
            }
        } else if (stride == 4) {
            if (channels == 1) return MTL::PixelFormatR32Float;
            else if (channels == 2) return MTL::PixelFormatRG32Float;
            else if (channels == 4) return MTL::PixelFormatRGBA32Float;
        }

        stl::abort(
            "image not supported in metal with channels {} stride {} linear {}",
            channels, stride, linear
        );
        return MTL::PixelFormatRG8Unorm;
    }

    Image::Image(cref<Descriptor> desc) noexcept {
        width = desc.image->width;
        height = desc.image->height;
        mips = math::max(1uz, desc.image->pixels.size());
        if (desc.state == State::samplable && !desc.image->pixels.empty()) {
            for (auto i = 0; i < mips; ++i)
                host.push_back(make_desc<Buffer>({
                    .ptr = desc.image->pixels[i].data(),
                    .state = Buffer::State::visible,
                    .type = desc.type,
                    .size = desc.image->pixels[i].size(),
                }));
            desc.image->pixels.clear();
        }

        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto tesc = MTL::TextureDescriptor::alloc();
        tesc->setTextureType(MTL::TextureType2D);
        tesc->setPixelFormat(impl->format(*desc.image));
        tesc->setWidth(width);
        tesc->setHeight(height);
        tesc->setDepth(1);
        tesc->setMipmapLevelCount(mips);
        tesc->setArrayLength(1);
        tesc->setSampleCount(1);
        device->newTexture(tesc);
    }

    Image::operator View() noexcept {
        return {this, {0, mips}, {0, 0}, {width, height}};
    }
}
