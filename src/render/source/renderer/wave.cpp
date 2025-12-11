#include "renderer.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/device/opaque/accel.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::renderer {
    auto Renderer::Impl::wave() noexcept -> void {
        command::Context::init();
        auto& stack = stl::stack::instance();
        auto& scheduler = stl::scheduler::instance();
        auto& vector = stl::vector<void>::instance();

        auto bsize = stack.bufs.size();
        auto vsize = vector.size();
        auto buffers = std::vector<obj<opaque::Buffer>>(bsize + vsize);

        scheduler.sync_parallel(uzv1{bsize}, [&](auto idx) {
            auto [i] = idx;
            auto& buf = stack.bufs[i];
            if (buf->idx != i) return;

            auto desc = opaque::Buffer::Descriptor{
                .type = command::Queue::Type::transfer,
                .state = opaque::Buffer::State::local,
                .ptr = buf->ptr,
                .size = buf->bytelen,
            };
            auto buffer = make_obj<opaque::Buffer>(desc);
            stl::stack::instance().release(buf);
            buf->ptr = mut<byte>(buffer->addr);
            buf->handle = uptr(buffer.get());
            buf->idx = math::maxv<u32>;
            buffers[i] = std::move(buffer);
        });

        auto sequence = std::vector<std::vector<byte>>(vsize);
        auto resources = std::vector<uptr>(vsize, 0);
        scheduler.sync_parallel(uzv1{vsize}, [&](auto idx) {
            auto [i] = idx;
            auto& vec = vector.storage[i];
            sequence[i] = vec.pack();
            if (sequence[i].empty()) return;

            auto desc = opaque::Buffer::Descriptor{
                .type = command::Queue::Type::transfer,
                .state = opaque::Buffer::State::local,
                .ptr = sequence[i].data(),
                .size = sequence[i].size(),
            };
            auto buffer = make_obj<opaque::Buffer>(desc);
            resources[i] = buffer->addr;
            buffers[bsize + i] = std::move(buffer);
        });

        auto& dividers = stl::vector<accel::Divider>::instance();
        auto& shapes = stl::vector<shape::Shape>::instance();
        auto primitives = std::vector<opaque::Acceleration::Primitive>{};
        auto instances = std::vector<opaque::Acceleration::Instance>{};
        for (auto i = 0; i < dividers.size(); ++i) {
            auto div = dividers[i];
            auto tlas = opaque::Acceleration::Instance{div->local_to_render->transform};
            auto blas = opaque::Acceleration::Primitive{};
            if (shapes.is<shape::Mesh>(div->shape))
                blas.mesh = shapes.get<shape::Mesh>(div->shape.index());
            else blas.aabb = div->shape->bounding_box(fm4{1.f}, 0);
            instances.push_back(tlas);
            primitives.push_back(blas);
        }
        auto accel = opaque::Acceleration{{
            .type = command::Queue::Type::render,
            .primitives = std::move(primitives),
            .instances = std::move(instances),
        }};
    }
}
