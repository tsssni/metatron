#pragma once

namespace mtt::stl {
    template<typename T, bool local = false>
    struct singleton {
        singleton() noexcept = default;
        singleton(cref<singleton>) = delete;
        singleton(rref<singleton>) = delete;
        auto operator=(cref<singleton>) -> ref<singleton> = delete;
        auto operator=(rref<singleton>) -> ref<singleton> = delete;

        auto static instance() noexcept -> ref<T> {
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
        inline_singleton(cref<inline_singleton>) = delete;
        inline_singleton(rref<inline_singleton>) = delete;
        auto operator=(cref<inline_singleton>) -> ref<inline_singleton> = delete;
        auto operator=(rref<inline_singleton>) -> ref<inline_singleton> = delete;

        inline static T inst{};
        auto static instance() noexcept -> ref<T> { return inst; }
    };

}
