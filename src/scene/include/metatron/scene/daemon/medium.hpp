#pragma once

namespace mtt::daemon {
    struct Medium_Daemon final {
        auto init() noexcept -> void;
        auto update() noexcept -> void;
    };
}
