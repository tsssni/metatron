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
            auto size = 0uz;
            auto offset = 0uz;
            auto idx = 0uz;
            auto aligned = ((index<Ts> > index<T> ? false : ((
                offset = math::align(size, alignof(Ts)),
                size = offset + sizeof(Ts),
            true))) && ...);
            return offset;
        }();

        auto constexpr static alignment = [] -> usize {
            return math::max(math::max(alignof(Ts)...), 4uz);
        }();

        // assure aggregate layout
        auto constexpr static size = [] -> usize {
            using U = type<sizeof...(Ts) - 1>;
            return offset<U> + sizeof(U);
        }();
    };
}
