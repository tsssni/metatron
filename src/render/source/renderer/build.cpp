#include <metatron/render/accel/accel.hpp>
#include <metatron/device/command/buffer.hpp>
#include <metatron/device/command/timeline.hpp>
#include <metatron/device/encoder/accel.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/device/opaque/accel.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::renderer {
    template<typename T>
    auto collect(
        ref<std::vector<opaque::Acceleration::Primitive>> primitives,
        ref<std::vector<u32>> counts
    ) noexcept -> void {
        auto& shapes = stl::vector<shape::Shape>::instance();
        for (auto i = 0; i < shapes.size<T>(); ++i)
            if constexpr (std::is_same_v<T, shape::Mesh>) {
                auto mesh = shapes.get<T>(i);
                primitives.push_back({
                    .type = opaque::Acceleration::Primitive::Type::mesh,
                    .mesh = shapes.get<T>(i),
                });
            } else {
                auto prim = opaque::Acceleration::Primitive{
                    .type = opaque::Acceleration::Primitive::Type::aabb,
                };
                auto t = math::Transform{fm4{1.f}};
                for (auto j = 0; j < shapes.get<T>(i)->size(); ++j)
                    prim.aabbs.push_back(shapes.get<T>(i)->bounding_box(t, j));
                primitives.push_back(prim);
            }
        counts.push_back(counts.back() + shapes.size<T>());
    }

    auto build(
        mut<command::Queue> queue,
        std::span<obj<command::Timeline>> uploads,
        mut<command::Timeline> timeline,
        ref<u64> count
    ) noexcept -> obj<opaque::Acceleration> {
        auto& scheduler = stl::scheduler::instance();
        auto& dividers = stl::vector<accel::Divider>::instance();
        auto& shapes = stl::vector<shape::Shape>::instance();
        auto counts = std::vector<u32>{0};
        auto primitives = std::vector<opaque::Acceleration::Primitive>{};
        auto instances = std::vector<opaque::Acceleration::Instance>{};

        collect<shape::Mesh>(primitives, counts);
        collect<shape::Sphere>(primitives, counts);

        for (auto i = 0; i < dividers.size(); ++i) {
            auto div = dividers[i];
            auto type = div->shape.type();
            auto idx = div->shape.index();
            auto tlas = opaque::Acceleration::Instance{
                .idx = counts[type] + idx,
                .transform = div->local_to_render->transform,
            };
            instances.push_back(tlas);
        }
        auto accel = make_desc<opaque::Acceleration>({
            .primitives = std::move(primitives),
            .instances = std::move(instances),
        });

        auto waits = command::Pairs(scheduler.size());
        for (auto i = 0; i < scheduler.size(); ++i)
            waits[i] = {uploads[i].get(), 1};
        auto cmd = queue->allocate(std::move(waits));

        auto transfer = encoder::Transfer_Encoder{cmd.get()};
        transfer.upload(*accel->instances);
        if (accel->bboxes) transfer.upload(*accel->bboxes);
        transfer.submit();

        auto builder = encoder::Acceleration_Encoder{cmd.get(), accel.get()};
        builder.build();
        builder.persist();
        builder.submit();
        queue->submit(std::move(cmd), {{timeline, ++count}});
        return accel;
    }
}
