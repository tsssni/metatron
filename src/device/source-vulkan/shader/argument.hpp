#pragma once
#include "../command/context.hpp"
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        vk::UniqueDescriptorSetLayout layout;
        vk::UniqueDescriptorSet set;
        Impl(std::string_view name) noexcept;
    };
}
