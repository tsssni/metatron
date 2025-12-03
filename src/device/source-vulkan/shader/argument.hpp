#pragma once
#include "../command/context.hpp"
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        vk::UniqueDescriptorSetLayout layout;
        vk::UniqueBuffer set;
        Impl(std::string_view name) noexcept;
    };
}
