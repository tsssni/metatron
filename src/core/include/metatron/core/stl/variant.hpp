#pragma once
#include <variant>

namespace mtt::stl {
    template<typename T, typename Variant>
    struct is_variant_alternative : std::false_type {};

    template<typename T, typename... Types>
    struct is_variant_alternative<T, std::variant<Types...>>
        : std::disjunction<std::is_same<T, Types>...> {};

    template<typename T, typename Variant>
    auto constexpr is_variant_alternative_v = is_variant_alternative<T, Variant>::value;
}
