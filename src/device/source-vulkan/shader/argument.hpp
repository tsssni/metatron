#pragma once
#include "../command/context.hpp"
#include "../opaque/buffer.hpp"
#include <metatron/device/shader/argument.hpp>
#include <metatron/core/stl/string.hpp>

namespace mtt::shader {
    struct Argument::Impl final {
        vk::UniqueDescriptorSetLayout layout;
        std::vector<usize> offsets;
        auto barrier(cref<shader::Descriptor> desc, opaque::Barrier barrier) noexcept -> opaque::Barrier;
        template<typename T>
        auto bind(mut<Argument> args, std::string_view field, T view) noexcept -> void;
        template<typename T>
        auto bind(mut<Argument> args, std::string_view field, Bindless<T> view) noexcept -> void;
        template<typename T>
        auto acquire(mut<Argument> args, std::string_view field, T view) noexcept -> void;
        template<typename T>
        auto acquire(mut<Argument> args, std::string_view field, Bindless<T> view) noexcept -> void;
    };
}
