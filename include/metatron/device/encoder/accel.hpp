#pragma once
#include <metatron/device/opaque/accel.hpp>

namespace mtt::encoder {
    struct Acceleration_Encoder final: stl::capsule<Acceleration_Encoder> {
        mut<command::Buffer> cmd;
        mut<opaque::Acceleration> accel;
        struct Impl;
        Acceleration_Encoder(mut<command::Buffer> cmd, mut<opaque::Acceleration> accel) noexcept;

        auto submit() noexcept -> void;
        auto build() noexcept -> void;
        auto persist() noexcept -> void;
    };
}

