#pragma once
#include "../command/context.hpp"
#include "../opaque/buffer.hpp"
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        std::vector<byte> constants;
        std::vector<byte> buffer;
        obj<opaque::Buffer> uniform;
        obj<opaque::Buffer> set;
        vk::UniqueDescriptorSetLayout layout;
        Impl(std::string_view name) noexcept;
    };
}
