#pragma once

namespace mtt::daemon {
    struct Material_Daemon final {
        auto init() noexcept -> void;
        auto update() noexcept -> void;
    };
}
