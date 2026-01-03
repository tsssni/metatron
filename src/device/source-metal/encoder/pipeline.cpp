#include "pipeline.hpp"

namespace mtt::encoder {
    Pipeline_Encoder::Pipeline_Encoder(
        mut<command::Buffer> cmd, mut<shader::Pipeline> ppl
    ) noexcept: cmd(cmd), ppl(ppl) {}

    auto Pipeline_Encoder::submit() noexcept -> void {}
    auto Pipeline_Encoder::bind() noexcept -> void {}
    auto Pipeline_Encoder::dispatch(uzv3 grid) noexcept -> void {}
}
