#pragma once
#include <metatron/device/command/buffer.hpp>
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
    };
}
