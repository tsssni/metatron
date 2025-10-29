#pragma once
#include <metatron/core/stl/type_array.hpp>
#include <metatron/core/stl/singleton.hpp>
#include <vector>
#include <functional>

namespace mtt::stl {
    template<typename F, typename... Ts>
    struct poly_vector final: singleton<poly_vector<F, Ts...>> {
        using ts = type_array<Ts...>;
        using lambdas = std::array<std::function<auto (byte const* ptr) -> mut<F>>, sizeof...(Ts)>;
        using idx_t = u32;
        static constexpr std::array<usize, sizeof...(Ts)> size_ts = { sizeof(Ts)... };
        static lambdas reinterpret_ts;

        template<typename T, typename... Args>
        requires (ts::template contains<T> && std::is_constructible_v<T, Args...>)
        auto emplace_back(Args&&... args) noexcept -> idx_t {
            auto idx = ts::template index<T>;
            auto size = storage[idx].size();
            storage[idx].resize(size + sizeof(T));
            auto ptr = (T*)(storage[idx].data() + size);
            std::construct_at(ptr, std::forward<Args>(args)...);
            return ((idx & 0xff) << 24) | ((storage[idx].size() / sizeof(T)) & 0xffffff);
        }
        
        template<typename T>
        requires std::is_constructible_v<std::decay_t<T>, T>
        auto push_back(T&& x) noexcept -> idx_t {
            return emplace_back<T>(std::forward<T>(x));
        }

        auto operator[](idx_t i) noexcept -> mut<F> {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            auto ptr = storage[t].data() + size_ts[t] * idx;
            return reinterpret_ts[t](ptr);
        }

        auto at(idx_t i) const noexcept -> view<F> {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            auto ptr = storage[t].data() + size_ts[t] * idx;
            return reinterpret_ts[t](ptr);
        }

        template<typename T>
        requires ts::template contains<T>
        auto data() noexcept -> T* {
            return (T*)storage[ts::template index<T>].data();
        }

        template<typename T>
        requires ts::template contains<T>
        auto size() noexcept -> usize {
            return storage[ts::template index<T>].size() / sizeof(T);
        }

    private:
        std::array<std::vector<byte>, sizeof...(Ts)> storage;
    };


    template<typename T>
    struct poly_vector<T>: poly_vector<T, T> {};

    template<typename F, typename... Ts>
    poly_vector<F, Ts...>::lambdas poly_vector<F, Ts...>::reinterpret_ts = {
        [] (auto ptr) { return (Ts*)ptr; }...
    };
}
