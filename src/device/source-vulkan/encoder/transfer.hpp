#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {
    struct Transfer_Encoder::Impl final {
        opaque::Barrier src_barrier;
        opaque::Barrier dst_barrier;

        template<typename T>
        auto persist(mut<Transfer_Encoder> encoder, T view) noexcept -> void;
        template<typename T>
        auto transfer(mut<Transfer_Encoder> encoder, u32 family, T view) noexcept -> void;
        template<typename T, typename U>
        auto copy(mut<Transfer_Encoder> encoder, T to, U from) noexcept -> void;
        template<typename T>
        auto copy(mut<Transfer_Encoder> encoder, T to, T from) noexcept -> void;
    };
}
