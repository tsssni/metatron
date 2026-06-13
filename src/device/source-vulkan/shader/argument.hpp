#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        vk::UniqueDescriptorSetLayout layout;
        std::vector<usize> offsets;
    };
}
