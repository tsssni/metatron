#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/resource/shape/mesh.hpp>

namespace mtt::opaque {
    struct Acceleration final: stl::capsule<Acceleration> {
        struct Primitive final {
            enum struct Type {
                mesh,
                aabb,
            } type;
            shape::Mesh const* mesh;
            std::vector<math::Bounding_Box> aabbs;
        };
        struct Instance final {
            u32 idx;
            fm4 transform;
        };

        std::vector<std::unique_ptr<Buffer>> buffers;
        std::vector<std::unique_ptr<Buffer>> scratches;
        std::unique_ptr<Buffer> bboxes;
        std::unique_ptr<Buffer> instances;

        struct Descriptor final {
            std::vector<Primitive> primitives;
            std::vector<Instance> instances;
            command::Type type = command::Type::render;
        };

        struct Impl;
        Acceleration(Descriptor const& desc) noexcept;
    };
}
