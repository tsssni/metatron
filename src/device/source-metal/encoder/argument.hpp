#pragma once
#include "../command/context.hpp"
#include <metatron/device/encoder/argument.hpp>

namespace mtt::encoder {
    struct Argument_Encoder::Impl final {
        template<typename T>
        auto identify(auto transform, T src, mut<shader::Argument> args, u32 base) noexcept -> void;
    };
}
