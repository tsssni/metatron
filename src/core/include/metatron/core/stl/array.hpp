#pragma once

namespace mtt::stl {
    namespace details {
        template<usize idx, typename... Ts>
        struct type_impl;

        template<typename T, typename... Ts>
        struct type_impl<0, T, Ts...> {
            using type = T;
        };

        template<usize idx, typename T, typename... Ts>
        struct type_impl<idx, T, Ts...> {
            using type = typename type_impl<idx - 1, Ts...>::type;
        };
    }

    template<typename... Ts>
    struct array final {
        template<typename T>
        static constexpr bool contains = (std::is_same_v<T, Ts> || ...);

        template<typename T>
        static constexpr usize index = [] {
            auto idx = 0;
            auto found = ((std::is_same_v<T, Ts> ? false : (++idx, true)) && ...);
            return idx;
        }();

        template<usize idx>
        using type = typename details::type_impl<idx, Ts...>::type;
    };
}
