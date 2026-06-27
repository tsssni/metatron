#pragma once
#include <metatron/device/opaque/accel.hpp>
#include <metatron/device/opaque/sampler.hpp>
#include <metatron/device/shader/argument.hpp>

namespace mtt::encoder {
    template<typename T>
    concept Set = std::is_aggregate_v<std::decay_t<T>> || std::is_scalar_v<std::decay_t<T>>;

    struct Argument_Encoder final: stl::capsule<Argument_Encoder> {
        mut<command::Buffer> cmd;
        mut<shader::Argument> args;
        struct Impl;
        Argument_Encoder(mut<command::Buffer> cmd, mut<shader::Argument> args) noexcept;

        auto submit() noexcept -> void;

        template<Set S>
        auto push(S&& set, uv2 range) noexcept -> void {
            push({view<byte>(&set), sizeof(set)}, range);
        }
        auto push(std::span<byte const> set, uv2 range) noexcept -> void;

        template<Set S, typename L, typename V = typename std::decay_t<L>::value_type>
        auto push(S&& set, u32 offset, L&& bindless) noexcept -> void {
            using Type = shader::Descriptor::Type;
            auto desc = args->reflection.back();
            auto type = Type{};
            if constexpr (std::is_same_v<V, opaque::Image::View>) type = Type::image;
            else if constexpr (std::is_same_v<V, opaque::Grid::View>) type = Type::grid;
            if (desc.type != type || desc.count != 0) stl::abort("set does not have bindless resources");
            push({view<byte>(&set), sizeof(set)}, offset, {view<byte>(bindless.data()), bindless.size() * sizeof(V)});
        }
        auto push(std::span<byte const> set, u32 offset, std::span<byte const> bindless) noexcept -> void;
    };
}
