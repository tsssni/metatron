#include "accel.hpp"

namespace mtt::encoder {
    Acceleration_Encoder::Acceleration_Encoder(
        mut<command::Buffer> cmd, mut<opaque::Acceleration> accel
    ) noexcept: cmd(cmd), accel(accel) {}

    auto Acceleration_Encoder::submit() noexcept -> void {}
    auto Acceleration_Encoder::build() noexcept -> void {}
    auto Acceleration_Encoder::persist() noexcept -> void {}
}
