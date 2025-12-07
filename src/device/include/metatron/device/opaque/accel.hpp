#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/bounding-box.hpp>

namespace mtt::opaque {
    struct Acceleration final {
        struct BLAS final {
            enum struct Type {
                mesh,
                aabb,
            } type;
            union {
                struct {
                    std::span<uv3 const> indices;
                    std::span<fv3 const> vertices;
                } mesh;
                math::Bounding_Box aabb;
            };
        };
        struct TLAS final { fm4 transform; };

        command::Queue::Type type;
        u64 timestamp = 0;

        struct Descriptor final {
            command::Queue::Type type;
            std::vector<BLAS> blases;
            std::vector<TLAS> tlases;
        };

        struct Impl;
        Acceleration(cref<Descriptor> desc) noexcept;
    };
}
