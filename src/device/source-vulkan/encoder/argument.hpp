#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/encoder/argument.hpp>

namespace mtt::encoder {
    struct Argument_Encoder::Impl final {
        auto update(cref<shader::Descriptor> desc, opaque::Barrier barrier) noexcept -> opaque::Barrier;
        template<typename T>
        auto bind(mut<Argument_Encoder> encoder, std::string_view field, T view) noexcept -> void;
        template<typename T>
        auto bind(mut<Argument_Encoder> encoder, std::string_view field, shader::Bindless<T> view) noexcept -> void;
        template<typename T>
        auto acquire(mut<Argument_Encoder> encoder, std::string_view field, T view) noexcept -> void;
        template<typename T>
        auto acquire(mut<Argument_Encoder> encoder, std::string_view field, shader::Bindless<T> view) noexcept -> void;
    };
}
