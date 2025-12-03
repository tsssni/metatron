#pragma once
#include "../command/context.hpp"
#include "../opaque/buffer.hpp"
#include <metatron/device/shader/argument.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        std::array<std::vector<byte>, command::Context::Impl::ring_count> buffer;
        std::array<obj<opaque::Buffer>, command::Context::Impl::ring_count> imported;
        obj<opaque::Buffer> set;
        vk::UniqueDescriptorSetLayout layout;
        Impl(std::string_view name) noexcept;
    };
}
