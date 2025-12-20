#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>

namespace mtt::renderer {
    struct Resources final {
        std::vector<obj<opaque::Buffer>> buffers;
        std::vector<obj<opaque::Buffer>> vectors{};
        obj<opaque::Buffer> resources;
        std::vector<obj<opaque::Image>> images;
        std::vector<obj<opaque::Grid>> grids;
    };
}
