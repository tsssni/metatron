#pragma once
#include <metatron/core/stl/type_array.hpp>
#include <metatron/core/stl/singleton.hpp>
#include <vector>
#include <functional>
#include <typeindex>

namespace mtt::stl {
    template<typename F>
    struct poly_vector final: singleton<poly_vector<F>> {
        using idx_t = u32;

        template<typename T>
        requires std::is_constructible_v<mut<F>, mut<T>>
        auto emplace_type() noexcept -> void {
            auto idx = map.size();
            map[typeid(T)] = idx;
            length.push_back(sizeof(T));
            reinterpreter.push_back([] (auto ptr) {
                return (T*)ptr;
            });
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> idx_t {
            auto idx = map[typeid(T)];
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
            auto ptr = storage[t].data() + length[t] * idx;
            return reinterpreter[t](ptr);
        }

        auto at(idx_t i) const noexcept -> view<F> {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            auto ptr = storage[t].data() + length[t] * idx;
            return reinterpreter.at(t)(ptr);
        }

        template<typename T>
        auto data() noexcept -> T* {
            return (T*)storage[map[typeid(T)]].data();
        }

        template<typename T>
        auto size() noexcept -> usize {
            return storage[map.at(typeid(T))].size() / sizeof(T);
        }

    private:
        std::vector<std::vector<byte>> storage;
        std::vector<idx_t> length;
        std::vector<std::function<auto (byte const* ptr) -> mut<F>>> reinterpreter;
        std::unordered_map<std::type_index, idx_t> map;
    };


    template<typename F>
    struct proxy final {
        using vec = poly_vector<F>;

        proxy(vec::idx_t idx): idx(idx) {}

        auto operator->() -> mut<F> {
            return vec::instance()[idx];
        }

        auto operator->() const -> view<F> {
            return vec::instance().at(idx);
        }

    private:
        vec::idx_t idx;
    };
}
