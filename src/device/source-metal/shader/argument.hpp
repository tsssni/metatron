#pragma once
#include "../command/context.hpp"
#include "../opaque/buffer.hpp"
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        obj<opaque::Buffer> parameters;
        std::vector<usize> offsets;
    };
}
