#pragma once
#include <metatron/core/stl/vector.hpp>

namespace mtt::stl {
    template<typename T>
    concept is_polynomial = requires { typename T::polynomial_marker; };

    template<typename Self, typename... Ts>
    struct polynomial {
        using polynomial_marker = u32;
        using ts = stl::array<Ts...>;
        using vs = stl::vector<Ts...>;
        tag<Ts...> idx;

        polynomial() noexcept = default;
        polynomial(u32 raw) noexcept: idx(raw) {}
        polynomial(tag<Ts...> idx) noexcept: idx(idx) {}

        auto static entity(std::string_view path) noexcept -> Self {
            return Self{vs::instance().entity(path)};
        }

        auto path() const noexcept -> std::string_view {
            auto sv = std::string_view{};
            auto _ = ((idx.storage() == vs::instance().template storage<Ts>()
            ? (sv = vs::instance().template path<Ts>(idx), true) : false) || ...);
            return sv;
        }

        template<typename T>
        auto static push(std::string_view path, rref<T> x) noexcept -> Self {
            return Self{vs::instance().template push<T>(path, std::move(x))};
        }
        template<typename T>
        auto static push(std::string_view path, cref<T> x) noexcept -> Self {
            return Self{vs::instance().template push<T>(path, x)};
        }

        template<typename T>
        auto static push_back(rref<T> x) noexcept -> Self {
            return Self{vs::instance().template push_back<T>(std::move(x))};
        }
        template<typename T>
        auto static push_back(cref<T> x) noexcept -> Self {
            return Self{vs::instance().template push_back<T>(x)};
        }

        template<typename T>
        auto is() const noexcept -> bool {
            return idx.storage() == vs::instance().template storage<T>();
        }

        operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return (bool)idx; }

        template<typename S, typename F>
        auto constexpr visit(this S&& self, F&& f) -> decltype(auto) {
            using R = decltype(f(self.idx.data()));
            if constexpr (std::is_void_v<R>) {
                auto _ = ((self.idx.storage() == vs::instance().template storage<Ts>()
                ? (f(self.idx.template data<Ts>()), true) : false) || ...);
            } else if constexpr (std::is_reference_v<R>) {
                using P = std::remove_reference_t<R>*;
                auto p = (P)nullptr;
                auto _ = ((self.idx.storage() == vs::instance().template storage<Ts>()
                ? (p = &f(self.idx.template data<Ts>()), true) : false) || ...);
                return static_cast<R>(*p);
            } else {
                auto r = R{};
                auto _ = ((self.idx.storage() == vs::instance().template storage<Ts>()
                ? (r = f(self.idx.template data<Ts>()), true) : false) || ...);
                return r;
            }
        }
    };

    template<typename T>
    concept is_variant = requires(T t) { typename T::variant_marker; };

    template<typename Self, typename... Ts>
    requires (sizeof...(Ts) > 0)
    struct alignas(stl::array<Ts...>::alignment) variant {
        using variant_marker = u32;
        using ts = stl::array<Ts...>;

        std::array<byte, ts::size> storage;
        u32 idx = math::maxv<u32>;

        variant() noexcept = default;

        template<typename T>
        requires ts::template contains<std::decay_t<T>>
        variant(T&& x) noexcept { emplace<std::decay_t<T>>(std::forward<T>(x)); }

        variant(cref<variant>) noexcept = delete;
        variant(rref<variant> rhs) noexcept {
            idx = rhs.idx;
            if (idx != math::maxv<u32>) auto _ = ((
            idx == ts::template index<Ts>
            ? (std::construct_at(get<Ts>(), std::move(*rhs.template get<Ts>())), true)
            : false
            ) || ...);
            rhs.idx = math::maxv<u32>;
        }
        ~variant() noexcept { destroy(); }

        auto operator=(cref<variant>) noexcept -> variant& = delete;
        auto operator=(rref<variant> rhs) noexcept -> variant& {
            destroy();
            std::construct_at(this, std::move(rhs));
            return *this;
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...> && ts::template contains<T>
        auto emplace(Args&&... args) noexcept -> void {
            destroy();
            idx = ts::template index<T>;
            std::construct_at(get<T>(), std::forward<Args>(args)...);
        }

        template<typename T>
        requires ts::template contains<std::decay_t<T>>
        auto push(T&& x) noexcept -> void {
            emplace<std::decay_t<T>>(std::forward<T>(x));
        }

        template<typename T>
        auto is() const noexcept -> bool { return ts::template index<T> == idx; }
        auto path() const noexcept -> std::string_view { return {}; }
        template<typename T>
        auto get() noexcept -> mut<T> { return mut<T>(&storage[ts::template offset<T>]); }
        template<typename T>
        auto get() const noexcept -> view<T> { return view<T>(&storage[ts::template offset<T>]); }
        auto index() const noexcept -> u32 { return idx; }
        auto size() const noexcept -> usize { return storage.size(); }

        operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return idx != math::maxv<u32>; }

        template<typename S, typename F>
        auto constexpr visit(this S&& self, F&& f) -> decltype(auto) {
            using R = decltype(f(self.template get<typename ts::template type<0>>()));
            if constexpr (std::is_void_v<R>) {
                auto _ = ((self.idx == ts::template index<Ts>
                ? (f(self.template get<Ts>()), true) : false) || ...);
            } else if constexpr (std::is_reference_v<R>) {
                using P = std::remove_reference_t<R>*;
                auto p = (P)nullptr;
                auto _ = ((self.idx == ts::template index<Ts>
                ? (p = &f(self.template get<Ts>()), true) : false) || ...);
                return static_cast<R>(*p);
            } else {
                auto r = R{};
                auto _ = ((self.idx == ts::template index<Ts>
                ? (r = f(self.template get<Ts>()), true) : false) || ...);
                return r;
            }
        }

    private:
        auto destroy() noexcept -> void {
            if (idx != math::maxv<u32>) auto _ = ((
                idx == ts::template index<Ts>
                ? (std::destroy_at(get<Ts>()), true) : false
            ) || ...);
        }
    };

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
