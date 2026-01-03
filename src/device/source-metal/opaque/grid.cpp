#include "grid.hpp"
#include "../command/allocator.hpp"

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
        auto& allocator = command::Allocator::instance();

        auto tesc = MTL::TextureDescriptor::alloc()->init();
        tesc->setTextureType(MTL::TextureType3D);
        tesc->setPixelFormat(MTL::PixelFormatR32Float);
        tesc->setWidth(width);
        tesc->setHeight(height);
        tesc->setDepth(depth);
        tesc->setMipmapLevelCount(1);
        tesc->setArrayLength(1);
        tesc->setSampleCount(1);
        tesc->setResourceOptions(MTL::ResourceStorageModePrivate);
        tesc->setUsage(desc.state == State::readonly ? MTL::TextureUsageShaderRead
        : MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite);

        auto sa = device->heapTextureSizeAndAlign(tesc);
        auto alloc = allocator.allocate(u32(command::Memory::Impl::Type::local), 0, sa.align, sa.size);
        auto heap = alloc.memory->impl->heap.get();
        impl->texture = heap->newTexture(tesc, alloc.offset);
    }

    Grid::operator View() noexcept {
        return {this, {0}, {width, height, depth}};
    }
}
