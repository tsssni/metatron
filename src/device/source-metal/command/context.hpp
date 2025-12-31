#pragma once
#include <metatron/device/command/context.hpp>
#include <Metal/Metal.hpp>

namespace mtt::command {
    struct Context::Impl final {
        mut<MTL::Device> device;
        Impl() noexcept;
    };
}
