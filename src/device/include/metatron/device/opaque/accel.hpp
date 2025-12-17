#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/bounding-box.hpp>

namespace mtt::opaque {
    struct Acceleration final {
        struct Primitive final {
            enum struct Type {
                mesh,
                aabb,
            } type;
            union {
                view<shape::Mesh> mesh;
                math::Bounding_Box aabb;
            };
        };
        struct Instance final { fm4 transform; };

        u64 timestamp = 0;

        std::vector<obj<Buffer>> buffers;
        std::vector<obj<Buffer>> scratches;

        struct Descriptor final {
            std::vector<Primitive> primitives;
            std::vector<Instance> instances;
            command::Type type = command::Type::render;
        };

        struct Impl;
        Acceleration(cref<Descriptor> desc) noexcept;
    };
}
