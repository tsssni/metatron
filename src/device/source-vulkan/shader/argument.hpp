#pragma once
#include "../command/context.hpp"
#include "../opaque/buffer.hpp"
#include <metatron/device/shader/argument.hpp>
#include <metatron/core/stl/string.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        vk::UniqueDescriptorSetLayout layout;
        std::vector<usize> offsets;
    };
}
