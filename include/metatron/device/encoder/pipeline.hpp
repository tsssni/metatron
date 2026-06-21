#pragma once
#include <metatron/device/shader/pipeline.hpp>

namespace mtt::encoder {
    struct Pipeline_Encoder final: stl::capsule<Pipeline_Encoder> {
        mut<command::Buffer> cmd;
        mut<shader::Pipeline> ppl;
        struct Impl;
        Pipeline_Encoder(mut<command::Buffer> cmd, mut<shader::Pipeline> ppl) noexcept;

        auto submit() noexcept -> void;
        auto bind() noexcept -> void;
        auto dispatch(uv3 threads, uv3 group) noexcept -> void;

        template<typename T>
        requires std::is_aggregate_v<std::decay_t<T>> || std::is_scalar_v<std::decay_t<T>>
        auto push(T&& constants) noexcept -> void {
            push({view<byte>(&constants), sizeof(constants)});
        }
        auto push(std::span<byte const> uniform) noexcept -> void;
    };
}
