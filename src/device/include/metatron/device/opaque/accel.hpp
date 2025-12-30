#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/bounding-box.hpp>

namespace mtt::opaque {
    struct Acceleration final: stl::capsule<Acceleration> {
        struct Primitive final {
            enum struct Type {
                mesh,
                aabb,
            } type;
            view<shape::Mesh> mesh;
            std::vector<math::Bounding_Box> aabbs;
        };
        struct Instance final {
            u32 idx;
            fm4 transform;
        };

        std::vector<obj<Buffer>> buffers;
        std::vector<obj<Buffer>> scratches;
        obj<Buffer> bboxes;
        obj<Buffer> instances;

        struct Descriptor final {
            std::vector<Primitive> primitives;
            std::vector<Instance> instances;
            command::Type type = command::Type::render;
        };

        struct Impl;
        Acceleration(cref<Descriptor> desc) noexcept;
    };
}
