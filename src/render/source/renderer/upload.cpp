#include "resource.hpp"
#include <metatron/device/command/timeline.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/core/stl/thread.hpp>
#include <nanovdb/GridHandle.h>
#include <barrier>

namespace mtt::renderer {
    auto upload(
        mut<command::Queue> queue,
        ref<std::vector<obj<command::Timeline>>> timelines
    ) noexcept -> Resources {
        auto& scheduler = stl::scheduler::instance();
        auto& stack = stl::stack::instance();
        auto& vector = stl::vector<void>::instance();

        auto& bidims = stl::vector<muldim::Image>::instance();
        auto& tridims = stl::vector<muldim::Grid>::instance();
        auto& nanodims = stl::vector<nanovdb::GridHandle<>>::instance();

        auto bsize = stack.bufs.size();
        auto vsize = vector.size();
        auto nsize = nanodims.size();
        auto isize = bidims.size();
        auto gsize = tridims.size();

        auto vecaddr = std::vector<uptr>(vsize, 0);
        auto voladdr = std::vector<uptr>(nsize, 0);
        auto vecarr = obj<opaque::Buffer>{};
        auto volarr = obj<opaque::Buffer>{};

        auto buffers = std::vector<obj<opaque::Buffer>>(bsize);
        auto vectors = std::vector<obj<opaque::Buffer>>(vsize);
        auto volumes = std::vector<obj<opaque::Buffer>>(nsize);
        auto images = std::vector<obj<opaque::Image>>(isize);
        auto grids = std::vector<obj<opaque::Grid>>(gsize);

        scheduler.sync_parallel(uzv1{scheduler.size()}, [
            &,
            barrier = std::make_shared<std::barrier<>>(scheduler.size()),
            bc = std::make_shared<std::atomic<u32>>(0),
            vc = std::make_shared<std::atomic<u32>>(0),
            nc = std::make_shared<std::atomic<u32>>(0),
            ic = std::make_shared<std::atomic<u32>>(0),
            gc = std::make_shared<std::atomic<u32>>(0)
        ](auto) {
            auto cmd = queue->allocate({});
            auto transfer = encoder::Transfer_Encoder{cmd.get()};

            auto i = 0;
            while ((i = bc->fetch_add(1, std::memory_order::relaxed)) < bsize) {
                auto& buf = stack.bufs[i];
                if (buf->idx != i) continue;

                auto buffer = make_desc<opaque::Buffer>({
                    .ptr = buf->ptr,
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = buf->bytelen,
                });
                transfer.upload(*buffer);
                transfer.persist(*buffer);
                stack.release(buf);

                buf->ptr = mut<byte>(buffer->addr);
                buf->handle = uptr(buffer.get());
                buf->idx = math::maxv<u32>;
                buffers[i] = std::move(buffer);
            }
            barrier->arrive_and_wait();

            i = 0;
            while ((i = vc->fetch_add(1, std::memory_order::relaxed)) < vsize) {
                if (false
                || i == bidims.index()
                || i == tridims.index()
                || i == nanodims.index()
                ) continue;
                auto& vec = vector.storage[i];
                auto sequence = vec.pack();
                if (sequence.empty()) continue;

                auto buffer = make_desc<opaque::Buffer>({
                    .ptr = sequence.data(),
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = sequence.size(),
                });
                transfer.upload(*buffer);
                transfer.persist(*buffer);
                vecaddr[i] = buffer->addr;
                vectors[i] = std::move(buffer);
            }
            barrier->arrive_and_wait();

            if (scheduler.index() == 0) {
                vecarr = make_desc<opaque::Buffer>({
                    .ptr = mut<byte>(vecaddr.data()),
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = vecaddr.size() * sizeof(uptr),
                });
                transfer.upload(*vecarr);
                transfer.persist(*vecarr);
            }

            i = 0;
            while ((i = nc->fetch_add(1, std::memory_order::relaxed)) < nsize) {
                auto vol = nanodims[i];
                auto buffer = make_desc<opaque::Buffer>({
                    .ptr = mut<byte>(vol->buffer().data()),
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = vol->bufferSize(),
                });
                transfer.upload(*buffer);
                transfer.persist(*buffer);
                voladdr[i] = buffer->addr;
                volumes[i] = std::move(buffer);
            }
            barrier->arrive_and_wait();

            if (scheduler.index() == 0 && voladdr.size() > 0) {
                volarr = make_desc<opaque::Buffer>({
                    .ptr = mut<byte>(voladdr.data()),
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = voladdr.size() * sizeof(uptr),
                });
                transfer.upload(*volarr);
                transfer.persist(*volarr);
            }

            i = 0;
            while ((i = ic->fetch_add(1, std::memory_order::relaxed)) < isize) {
                images[i] = make_desc<opaque::Image>({
                    .image = bidims[i],
                    .state = opaque::Image::State::samplable,
                    .type = command::Type::render,
                });
                transfer.upload(*images[i]);
                transfer.persist(*images[i]);
            }
            barrier->arrive_and_wait();

            i = 0;
            while ((i = gc->fetch_add(1, std::memory_order::relaxed)) < gsize) {
                grids[i] = make_desc<opaque::Grid>({
                    .grid = tridims[i],
                    .state = opaque::Grid::State::readonly,
                    .type = command::Type::render,
                });
                transfer.upload(*grids[i]);
                transfer.persist(*grids[i]);
            }
            barrier->arrive_and_wait();
            transfer.submit();

            timelines[scheduler.index()] = make_obj<command::Timeline>();
            queue->submit(std::move(cmd), {{timelines[scheduler.index()].get(), 1}});
        });

        return Resources{
            std::move(buffers),
            std::move(vectors),
            std::move(volumes),
            std::move(vecarr),
            std::move(volarr),
            std::move(images),
            std::move(grids),
        };
    }
}
