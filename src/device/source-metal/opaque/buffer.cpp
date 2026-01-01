#include "buffer.hpp"

namespace mtt::opaque {
    Buffer::Buffer(cref<Descriptor> desc) noexcept:
    size(desc.size),
    state(desc.state) {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();

        if (desc.state != State::local || desc.ptr)
            impl->host_buffer = device->newBuffer(desc.size, MTL::ResourceStorageModeShared);
        if (desc.state != State::visible)
            impl->device_buffer = device->newBuffer(desc.size, MTL::ResourceStorageModePrivate);
        if (impl->host_buffer) {
            ptr = mut<byte>(impl->host_buffer->contents());
            if (desc.ptr) std::memcpy(ptr, desc.ptr, desc.size);
        }
    }

    Buffer::Buffer(rref<Buffer> rhs) noexcept { *this = std::move(rhs); }
    auto Buffer::operator=(rref<Buffer> rhs) noexcept -> ref<Buffer> {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        state = rhs.state; ptr = rhs.ptr;
        addr = rhs.addr; size = rhs.size; dirty = std::move(rhs.dirty);
        impl = std::move(rhs.impl);
        rhs.ptr = nullptr;
        rhs.addr = 0;
        return *this;
    }

    Buffer::operator View() noexcept {
        return {this, 0, size};
    }
}
