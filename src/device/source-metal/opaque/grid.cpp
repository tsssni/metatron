#include "grid.hpp"

namespace mtt::opaque {
    Grid::Grid(cref<Descriptor> desc) noexcept {
        width = desc.grid->width;
        height = desc.grid->height;
        depth = desc.grid->depth;
        if (desc.state == State::readonly && !desc.grid->cells.empty()) {
            host = make_desc<Buffer>({
                .ptr = mut<byte>(desc.grid->cells.data()),
                .state = Buffer::State::visible,
                .type = desc.type,
                .size = desc.grid->cells.size() * sizeof(f32),
            });
            desc.grid->cells.clear();
        }

        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto tesc = MTL::TextureDescriptor::alloc();
        tesc->setTextureType(MTL::TextureType3D);
        tesc->setPixelFormat(MTL::PixelFormatR32Float);
        tesc->setWidth(width);
        tesc->setHeight(height);
        tesc->setDepth(depth);
        tesc->setMipmapLevelCount(1);
        tesc->setArrayLength(1);
        tesc->setSampleCount(1);
        device->newTexture(tesc);

    }

    Grid::operator View() noexcept {
        return {this, {0}, {width, height, depth}};
    }
}
