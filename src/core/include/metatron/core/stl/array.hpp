#pragma once
#include <metatron/core/math/bit.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::stl {
    template<typename... Ts>
    struct array final {
        template<usize idx>
        using type = std::tuple_element_t<idx, std::tuple<Ts...>>;

        template<typename T>
        auto constexpr static contains = (std::is_same_v<T, Ts> || ...);

        template<typename T>
        auto constexpr static index = [] -> usize {
            auto idx = 0;
            auto found = ((std::is_same_v<T, Ts> ? false : (++idx, true)) && ...);
            return idx;
        }();

        // assure aggregate layout
        template<typename T>
        auto constexpr static offset = [] -> usize {
            auto size = 0;
            auto offset = 0;
            auto idx = 0;
            auto aligned = ((index<Ts> > index<T> ? false : ((
                offset = math::align(size, alignof(Ts)),
                size = offset + sizeof(Ts),
            true))) && ...);
            return offset;
        }();

        // assure aggregate layout
        auto constexpr static size = [] -> usize {
            using U = type<sizeof...(Ts) - 1>;
            auto alignment = math::max(alignof(Ts)...);
            auto size = offset<U> + sizeof(U);
            return math::align(size, alignment);
        }();
    };
}
