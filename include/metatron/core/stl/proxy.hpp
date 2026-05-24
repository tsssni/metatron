#pragma once
#include <metatron/core/stl/vector.hpp>

namespace mtt::stl {
    template<typename T>
    concept is_proxy = requires(T t) { typename T::proxy_marker; };

    template<typename P, typename T>
    struct proxy {
        using proxy_marker = u32;
        using vs = mtt::stl::vector<T>;
        mtt::tag<T> idx;

        proxy() noexcept = default;
        proxy(u32 raw) noexcept: idx(raw) {}
        proxy(mtt::tag<T> idx) noexcept: idx(idx) {}

        auto operator->() noexcept { return idx.operator->(); }
        auto operator->() const noexcept { return idx.operator->(); }
        auto operator*() noexcept -> ref<T> { return *idx; }
        auto operator*() const noexcept -> cref<T> { return *idx; }

        operator cref<T>() const noexcept { return *idx; }
        operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return (bool)idx; }

        auto path() const noexcept -> std::string_view {
            return mtt::stl::vector<T>::instance().path(idx);
        }

        auto static entity(std::string_view path) noexcept -> P {
            return P{vs::instance().entity(path)};
        }
    };
}
