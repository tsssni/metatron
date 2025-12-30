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
}
