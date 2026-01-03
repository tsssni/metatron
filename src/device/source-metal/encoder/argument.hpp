#pragma once
#include "../command/context.hpp"
#include <metatron/device/encoder/argument.hpp>

namespace mtt::encoder {
    struct Argument_Encoder::Impl final {
        template<typename T>
        auto bind(mut<Argument_Encoder> encoder, std::string_view field, view<T> resource) noexcept -> void;
        template<typename T>
        auto bind(mut<Argument_Encoder> encoder, shader::Bindless<T> bindless) noexcept -> void;
    };
}
