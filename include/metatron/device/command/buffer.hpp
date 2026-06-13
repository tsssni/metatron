#pragma once
#include <metatron/device/command/block.hpp>

namespace mtt::command {
    struct Buffer final: stl::capsule<Buffer> {
        Type type;
        Blocks blocks;
        std::vector<obj<opaque::Buffer>> stages;

        struct Impl;
        Buffer() noexcept;

    private:
        friend Queue;
        std::vector<Pair> waits;
        std::vector<Pair> signals;
    };
}
