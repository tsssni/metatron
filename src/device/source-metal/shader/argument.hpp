#pragma once
#include "../command/context.hpp"
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        std::vector<usize> offsets;
    };
}
