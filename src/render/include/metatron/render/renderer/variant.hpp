#pragma once
#include <metatron/render/scene/descriptor.hpp>
#include <metatron/core/stl/variant.hpp>
#include <variant>

namespace mtt::renderer {
    template<typename... Ts>
    struct underlying_variant;

    template<pro::facade F, typename... Ts>
    struct underlying_variant<stl::variant<F, Ts...>> final {
        using type = std::variant<scene::descriptor_t<Ts>...>;
    };

    template<typename T>
    using underlying_variant_t = underlying_variant<T>::type;
}
