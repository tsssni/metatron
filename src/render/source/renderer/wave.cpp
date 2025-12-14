#include "renderer.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/device/command/buffer.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/accel.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::renderer {
    auto Renderer::Impl::wave() noexcept -> void {
        command::Context::init();
        auto& scheduler = stl::scheduler::instance();
        auto upload_timelines = std::vector<obj<command::Timeline>>(scheduler.size());
        auto render_timeline = make_obj<command::Timeline>();
        auto transfer_timeline = make_obj<command::Timeline>();
        auto network_timline = make_obj<command::Timeline>();
        auto render_count = u64{0};
        auto tranfer_count = u64{0};
        auto network_count = u64{0};

        auto buffers = std::vector<obj<opaque::Buffer>>{};
        auto vectors = std::vector<obj<opaque::Buffer>>{};
        auto resources = obj<opaque::Buffer>{};
        auto images = std::vector<obj<opaque::Image>>{};
        auto grids = std::vector<obj<opaque::Grid>>{};

        auto render_queue = make_obj<command::Queue>(command::Queue::Type::render);
        auto transfer_queue = make_obj<command::Queue>(command::Queue::Type::transfer);

        [&] {
            auto& stack = stl::stack::instance();
            auto& vector = stl::vector<void>::instance();
            auto& textures = stl::vector<muldim::Image>::instance();
            auto& volumes = stl::vector<muldim::Grid>::instance();
            auto vsize = vector.size();
            auto bsize = stack.bufs.size();
            auto isize = textures.size();
            auto gsize = volumes.size();
            auto sequence = std::vector<std::vector<byte>>(vsize);
            auto addresses = std::vector<uptr>(vsize, 0);
            buffers = std::vector<obj<opaque::Buffer>>(bsize);
            vectors = std::vector<obj<opaque::Buffer>>(vsize);
            images = std::vector<obj<opaque::Image>>(isize);
            grids = std::vector<obj<opaque::Grid>>(gsize);

            scheduler.sync_parallel(uzv1{scheduler.size()}, [
                &,
                bc = std::make_shared<std::atomic<u32>>(0),
                vc = std::make_shared<std::atomic<u32>>(0),
                ic = std::make_shared<std::atomic<u32>>(0),
                gc = std::make_shared<std::atomic<u32>>(0)
            ](auto) {
                auto cmd = render_queue->allocate();
                auto transfer = encoder::Transfer_Encoder{cmd.get()};

                auto i = 0;
                while ((i = bc->fetch_add(1, std::memory_order::acq_rel)) < bsize) {
                    auto& buf = stack.bufs[i];
                    if (buf->idx != i) continue;

                    auto desc = opaque::Buffer::Descriptor{
                        .cmd = cmd.get(),
                        .ptr = buf->ptr,
                        .state = opaque::Buffer::State::local,
                        .size = buf->bytelen,
                    };
                    auto buffer = make_obj<opaque::Buffer>(desc);
                    transfer.upload(*buffer);
                    stl::stack::instance().release(buf);
                    buf->ptr = mut<byte>(buffer->addr);
                    buf->handle = uptr(buffer.get());
                    buf->idx = math::maxv<u32>;
                    buffers[i] = std::move(buffer);
                }

                i = 0;
                while ((i = vc->fetch_add(1, std::memory_order::acq_rel)) < vsize) {
                    auto& vec = vector.storage[i];
                    sequence[i] = vec.pack();
                    if (sequence[i].empty()) continue;

                    auto desc = opaque::Buffer::Descriptor{
                        .cmd = cmd.get(),
                        .ptr = sequence[i].data(),
                        .state = opaque::Buffer::State::local,
                        .size = sequence[i].size(),
                    };
                    auto buffer = make_obj<opaque::Buffer>(desc);
                    transfer.upload(*buffer);
                    addresses[i] = buffer->addr;
                    vectors[i] = std::move(buffer);
                }

                if (scheduler.index() == 0) {
                    auto desc = opaque::Buffer::Descriptor{
                        .cmd = cmd.get(),
                        .ptr = mut<byte>(addresses.data()),
                        .state = opaque::Buffer::State::local,
                        .size = addresses.size() * sizeof(uptr),
                    };
                    resources = make_obj<opaque::Buffer>(desc);
                    transfer.upload(*resources);
                }
                
                i = 0;
                while ((i = ic->fetch_add(1, std::memory_order::acq_rel)) < isize) {
                    images[i] = make_obj<opaque::Image>(
                    opaque::Image::Descriptor{
                        .cmd = cmd.get(),
                        .image = textures[i],
                        .state = opaque::Image::State::samplable,
                    });
                    transfer.upload(*images[i]);
                }

                i = 0;
                while ((i = gc->fetch_add(1, std::memory_order::acq_rel)) < gsize) {
                    grids[i] = make_obj<opaque::Grid>(
                    opaque::Grid::Descriptor{
                        .cmd = cmd.get(),
                        .grid = volumes[i],
                        .state = opaque::Grid::State::readonly,
                    });
                    transfer.upload(*grids[i]);
                }

                upload_timelines[scheduler.index()] = make_obj<command::Timeline>();
                cmd->signals = {{upload_timelines[scheduler.index()].get(), 1}};
                render_queue->submit(std::move(cmd));
            });
        }();

        auto accel = [&] {
            auto cmd = render_queue->allocate();
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
            auto accel = make_obj<opaque::Acceleration>(
            opaque::Acceleration::Descriptor{
                .primitives = std::move(primitives),
                .instances = std::move(instances),
            });
            for (auto i = 0; i < scheduler.size(); ++i)
                cmd->waits.push_back({upload_timelines[i].get(), 1});
            cmd->signals = {{render_timeline.get(), ++render_count}};
            render_queue->submit(std::move(cmd));
            return std::move(accel);
        }();
    }
}
