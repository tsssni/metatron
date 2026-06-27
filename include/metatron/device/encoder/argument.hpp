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

        template<Set S, typename C, typename M>
        auto push(S&& set, M C::* field) noexcept -> void {
            push(std::forward<S>(set), uv2{
                (u32)(view<byte>(&(set.*field)) - view<byte>(&set)),
                (u32)sizeof(M),
            });
        }
        template<Set S>
        auto push(S&& set) noexcept -> void {
            push(std::forward<S>(set), {0, sizeof(S)});
        }
        template<Set S>
        auto push(S&& set, uv2 range) noexcept -> void {
            push({view<byte>(&set), sizeof(set)}, range);
        }
        auto push(std::span<byte const> set, uv2 range) noexcept -> void;

        template<typename T, typename V = T::View, Set S>
        auto push(S&& set, std::tuple<u32, std::span<V>> bindless) noexcept -> void {
            using Type = shader::Descriptor::Type;
            auto [offset, span] = bindless;
            auto desc = args->reflection.back();
            auto type = Type{};
            if constexpr (std::is_same_v<T, opaque::Image>) type = Type::image;
            else if constexpr (std::is_same_v<T, opaque::Grid>) type = Type::grid;
            if (desc.type != type || desc.count != 0) stl::abort("set does not have bindless resources");
            push({view<byte>(&set), sizeof(set)}, {offset, {view<byte>(span.data()), span.size() * sizeof(V)}});
        }
        auto push(std::span<byte const> set, std::tuple<u32, std::span<byte const>> bindless) noexcept -> void;
    };
}
