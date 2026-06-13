#pragma once
#include <memory>
#include <type_traits>
#include <concepts>

namespace mtt::inline prelude {
    template<typename T>
    struct obj_impl final {
        using type = std::unique_ptr<T>;
    };

    template<typename T>
    using obj = obj_impl<T>::type;

    template<typename T, typename... Args>
    requires std::constructible_from<T, Args...>
    auto make_obj(Args&&... args) -> obj<T> {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    struct view_impl final {
        using type = std::conditional_t<
            std::is_const_v<T>,
            T const*,
            T*
        >;
    };

    template<typename T>
    using mut = view_impl<std::remove_const_t<T>>::type;

    template<typename T>
    using view = view_impl<std::add_const_t<std::remove_const_t<T>>>::type;

    template<typename T>
    using ref = T&;

    template<typename T>
    using cref = T const&;

    template<typename T>
    using rref = T&&;

    template<typename T>
    auto make_mut(ref<T> x) -> mut<T> {
        return &x;
    }

    template<typename T>
    auto make_view(cref<T> x) -> view<T> {
        return &x;
    }
}
