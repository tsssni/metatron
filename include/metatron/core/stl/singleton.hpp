#pragma once

namespace mtt::stl {
    template<typename T, bool local = false>
    struct singleton {
        singleton() noexcept = default;
        singleton(singleton const&) = delete;
        singleton(singleton&&) = delete;
        auto operator=(singleton const&) -> singleton& = delete;
        auto operator=(singleton&&) -> singleton& = delete;

        auto static instance() noexcept -> T& {
            if constexpr (local) {
                T thread_local instance;
                return instance;
            } else {
                T static instance;
                return instance;
            }
        }
    };

    // avoids magic-static atomic guard load on every access.
    template<typename T>
    struct inline_singleton {
        inline_singleton() noexcept = default;
        inline_singleton(inline_singleton const&) = delete;
        inline_singleton(inline_singleton&&) = delete;
        auto operator=(inline_singleton const&) -> inline_singleton& = delete;
        auto operator=(inline_singleton&&) -> inline_singleton& = delete;

        inline static T inst{};
        auto static instance() noexcept -> T& { return inst; }
    };

}
