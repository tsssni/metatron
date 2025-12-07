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
        auto blases = std::vector<opaque::Acceleration::BLAS>{};
        auto tlases = std::vector<opaque::Acceleration::TLAS>{};
        for (auto i = 0; i < dividers.size(); ++i) {
            auto div = dividers[i];
            auto tlas = opaque::Acceleration::TLAS{div->local_to_render->transform};
            auto blas = opaque::Acceleration::BLAS{};
            if (shapes.is<shape::Mesh>(div->shape)) {
                auto& mesh = meshes[div->shape.index()];
                blas.mesh = {
                    .indices = std::span<uv3 const>{mesh.indices},
                    .vertices = std::span<fv3 const>{mesh.vertices},
                };
            } else blas.aabb = div->shape->bounding_box(fm4{1.f}, 0);
            tlases.push_back(tlas);
            blases.push_back(blas);
        }
        auto accel = opaque::Acceleration{{
            .type = command::Queue::Type::transfer,
            .blases = blases,
            .tlases = tlases,
        }};
    }
}
