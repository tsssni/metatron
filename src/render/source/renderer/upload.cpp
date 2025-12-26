#include "resource.hpp"
#include <metatron/device/command/timeline.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/core/stl/thread.hpp>
#include <barrier>

namespace mtt::renderer {
    auto upload(
        mut<command::Queue> queue,
        ref<std::vector<obj<command::Timeline>>> timelines
    ) noexcept -> Resources {
        auto& scheduler = stl::scheduler::instance();
        auto& stack = stl::stack::instance();
        auto& vector = stl::vector<void>::instance();

        auto buffers = std::vector<obj<opaque::Buffer>>{};
        auto vectors = std::vector<obj<opaque::Buffer>>{};
        auto resources = obj<opaque::Buffer>{};
        auto images = std::vector<obj<opaque::Image>>{};
        auto grids = std::vector<obj<opaque::Grid>>{};

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
            barrier = std::make_shared<std::barrier<>>(scheduler.size()),
            bc = std::make_shared<std::atomic<u32>>(0),
            vc = std::make_shared<std::atomic<u32>>(0),
            ic = std::make_shared<std::atomic<u32>>(0),
            gc = std::make_shared<std::atomic<u32>>(0)
        ](auto) {
            auto cmd = queue->allocate();
            auto transfer = encoder::Transfer_Encoder{cmd.get()};

            auto i = 0;
            while ((i = bc->fetch_add(1, std::memory_order::relaxed)) < bsize) {
                auto& buf = stack.bufs[i];
                if (buf->idx != i) continue;

                auto desc = opaque::Buffer::Descriptor{
                    .ptr = buf->ptr,
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = buf->bytelen,
                };
                auto buffer = make_obj<opaque::Buffer>(desc);
                transfer.upload(*buffer);
                transfer.persist(*buffer);
                stack.release(buf);

                buf->ptr = mut<byte>(buffer->addr);
                buf->idx = math::maxv<u32>;
                buffers[i] = std::move(buffer);
            }
            barrier->arrive_and_wait();

            i = 0;
            while ((i = vc->fetch_add(1, std::memory_order::relaxed)) < vsize) {
                auto& vec = vector.storage[i];
                sequence[i] = vec.pack();
                if (sequence[i].empty()) continue;

                auto desc = opaque::Buffer::Descriptor{
                    .ptr = sequence[i].data(),
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = sequence[i].size(),
                };
                auto buffer = make_obj<opaque::Buffer>(desc);
                transfer.upload(*buffer);
                transfer.persist(*buffer);
                addresses[i] = buffer->addr;
                vectors[i] = std::move(buffer);
            }
            barrier->arrive_and_wait();

            if (scheduler.index() == 0) {
                auto desc = opaque::Buffer::Descriptor{
                    .ptr = mut<byte>(addresses.data()),
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = addresses.size() * sizeof(uptr),
                };
                resources = make_obj<opaque::Buffer>(desc);
                encoder::Transfer_Encoder{cmd.get()}.upload(*resources);
            }

            i = 0;
            while ((i = ic->fetch_add(1, std::memory_order::relaxed)) < isize) {
                images[i] = make_obj<opaque::Image>(
                opaque::Image::Descriptor{
                    .image = textures[i],
                    .state = opaque::Image::State::samplable,
                    .type = command::Type::render,
                });
                transfer.upload(*images[i]);
                transfer.persist(*images[i]);
            }
            barrier->arrive_and_wait();

            i = 0;
            while ((i = gc->fetch_add(1, std::memory_order::relaxed)) < gsize) {
                grids[i] = make_obj<opaque::Grid>(
                opaque::Grid::Descriptor{
                    .grid = volumes[i],
                    .state = opaque::Grid::State::readonly,
                    .type = command::Type::render,
                });
                transfer.upload(*grids[i]);
                transfer.persist(*grids[i]);
            }
            barrier->arrive_and_wait();

            timelines[scheduler.index()] = make_obj<command::Timeline>();
            cmd->signals = {{timelines[scheduler.index()].get(), 1}};
            queue->submit(std::move(cmd));
        });

        return Resources{
            std::move(buffers),
            std::move(vectors),
            std::move(resources),
            std::move(images),
            std::move(grids),
        };
    }
}
