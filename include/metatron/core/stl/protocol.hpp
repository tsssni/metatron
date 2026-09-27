#pragma once
#include <metatron/core/stl/vector.hpp>

namespace mtt::stl {
    template<typename T>
    concept polymorphic = requires { typename T::polymorphic_marker; };

    template<typename Self, typename... Ts>
    struct polymorph {
        using polymorphic_marker = u32;
        using ts = stl::array<Ts...>;
        using vs = stl::vector<Ts...>;
        tag<Ts...> idx;

        polymorph() noexcept = default;
        polymorph(u32 raw) noexcept: idx(raw) {}
        polymorph(tag<Ts...> idx) noexcept: idx(idx) {}

        auto static entity(std::string_view path) noexcept -> Self {
            return {vs::entity(path)};
        }

        auto path() const noexcept -> std::string_view {
            auto sv = std::string_view{};
            auto t = idx.type();
            auto _ = ((t == ts::template index<Ts> ? (
                sv = vs::template path<Ts>(idx)
            , true) : false) || ...);
            return sv;
        }

        template<typename T>
        auto static push(std::string_view path, T&& x) noexcept -> Self {
            return {vs::template push<T>(path, std::move(x))};
        }
        template<typename T>
        auto static push(std::string_view path, T const& x) noexcept -> Self {
            return {vs::template push<T>(path, x)};
        }

        template<typename T>
        auto static push_back(T&& x) noexcept -> Self {
            return {vs::template push_back<T>(std::move(x))};
        }
        template<typename T>
        auto static push_back(T const& x) noexcept -> Self {
            return {vs::template push_back<T>(x)};
        }

        template<typename T>
        auto is() const noexcept -> bool {
            return idx.type() == ts::template index<T>;
        }

        operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return (bool)idx; }

        template<typename S, typename F>
        auto constexpr visit(this S&& self, F&& f) -> decltype(auto) {
            using First = typename ts::template type<0>;
            using R = decltype(f(self.idx.template data<First>()));
            using SS = std::remove_reference_t<S>;
            using thunk_t = R(*)(SS&, F&);
            return [&]<usize... Is>(std::index_sequence<Is...>) -> R {
                auto constexpr table = std::to_array({
                    +[](SS& s, F& f) -> R {
                        return f(s.idx.template data<typename ts::template type<Is>>());
                    }...
                });
                return table[self.idx.type()](self, f);
            }(std::make_index_sequence<sizeof...(Ts)>{});
        }
    };

    template<typename T>
    concept proxied = requires(T t) { typename T::proxy_marker; };

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
        auto operator*() noexcept -> T& { return *idx; }
        auto operator*() const noexcept -> T const& { return *idx; }

        operator T const&() const noexcept { return *idx; }
        operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return (bool)idx; }

        auto path() const noexcept -> std::string_view {
            return mtt::stl::vector<T>::path(idx);
        }

        auto static entity(std::string_view path) noexcept -> P {
            return {vs::entity(path)};
        }
    };

    template<typename T>
    concept sealed = requires { typename T::tag_marker; };

    template<template<typename> typename S, typename... Ts>
    requires (S<Ts>::value && ...)
    using seal = mtt::tag<Ts...>;

    struct cartesian final {
        template<typename F, typename... Ts>
        requires (sealed<Ts> && ...)
        auto operator()(F&& f, Ts... tags) const noexcept -> void { visit(f, tags...); }

    private:
        template<typename... Us, typename F>
        auto visit(F& f) const noexcept -> void { f.template operator()<Us...>(); }

        template<typename... Us, typename F, typename T, typename... Ts>
        auto visit(F& f, T tag, Ts... tags) const noexcept -> void {
            [&]<typename... Vs>(std::type_identity<stl::array<Vs...>>) { auto _ =
            ((tag.type() == T::ts::template index<Vs> ? (
                this->template visit<Us..., Vs>(f, tags...)
            , true) : false) || ...); }(std::type_identity<typename T::ts>{});
        }
    };
}
