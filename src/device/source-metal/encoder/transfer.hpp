#pragma once
#include "../command/context.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {
    struct Transfer_Encoder::Impl final {
        mut<MTL::BlitCommandEncoder> encoder;
        template<typename T, typename U>
        auto copy(mut<Transfer_Encoder> encoder, T to, U from) noexcept -> void;
    };
}
