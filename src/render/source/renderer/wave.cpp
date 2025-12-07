#include "renderer.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/device/opaque/accel.hpp>
#include <metatron/resource/shape/mesh.hpp>

namespace mtt::renderer {
    auto Renderer::Impl::wave() noexcept -> void {
        command::Context::init();

        auto& dividers = stl::vector<accel::Divider>::instance();
        auto& shapes = stl::vector<shape::Shape>::instance();
        auto meshes = shapes.data<shape::Mesh>();
        auto primitives = std::vector<opaque::Acceleration::Primitive>{};
        auto instances = std::vector<opaque::Acceleration::Instance>{};
        for (auto i = 0; i < dividers.size(); ++i) {
            auto div = dividers[i];
            auto tlas = opaque::Acceleration::Instance{div->local_to_render->transform};
            auto blas = opaque::Acceleration::Primitive{};
            if (shapes.is<shape::Mesh>(div->shape))
                blas.mesh = &meshes[div->shape.index()];
            else blas.aabb = div->shape->bounding_box(fm4{1.f}, 0);
            instances.push_back(tlas);
            primitives.push_back(blas);
        }
        auto accel = opaque::Acceleration{{
            .type = command::Queue::Type::transfer,
            .primitives = std::move(primitives),
            .instances = std::move(instances),
        }};
    }
}
