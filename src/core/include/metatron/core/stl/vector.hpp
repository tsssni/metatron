#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/math/constant.hpp>
#include <vector>
#include <functional>
#include <typeindex>

namespace mtt::stl {
    template<typename F>
    struct poly_vector final: singleton<poly_vector<F>> {
        template<typename T>
        requires std::is_constructible_v<mut<F>, mut<T>>
        auto emplace_type() noexcept -> void {
            auto idx = map.size();
            map[typeid(T)] = idx;
            storage.push_back({});
            length.push_back(sizeof(T));
            reinterpreter.push_back([] (byte const* ptr) {
                return make_view<F>(*(T*)ptr);
            });
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            auto idx = map[typeid(T)];
            auto size = storage[idx].size();
            storage[idx].resize(size + sizeof(T));
            auto ptr = (T*)(storage[idx].data() + size);
            std::construct_at(ptr, std::forward<Args>(args)...);
            return ((idx & 0xff) << 24) | ((size / sizeof(T)) & 0xffffff);
        }
        
        template<typename T>
        requires std::is_constructible_v<std::decay_t<T>, T>
        auto push_back(T&& x) noexcept -> u32 {
            return emplace_back<T>(std::forward<T>(x));
        }

        auto operator[](u32 i) noexcept -> mut<F> {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            auto ptr = storage[t].data() + length[t] * idx;
            return reinterpreter[t](ptr);
        }

        auto at(u32 i) const noexcept -> view<F> {
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
        std::vector<u32> length;
        std::vector<std::function<auto (byte const* ptr) -> mut<F>>> reinterpreter;
        std::unordered_map<std::type_index, u32> map;
    };


    template<typename F>
    struct proxy final {
        using vec = poly_vector<F>;

        proxy(): idx(math::maxv<u32>) {};
        proxy(u32 idx): idx(idx) {}

        auto data() -> mut<F> {
            return vec::instance()[idx];
        }

        auto data() const -> view<F> {
            return vec::instance().at(idx);
        }

        auto operator->() -> mut<F> {
            return data();
        }

        auto operator->() const -> view<F> {
            return data();
        }

        explicit operator u32() const {
            return idx;
        }

        operator bool() const {
            return idx != math::maxv<u32>;
        }

    private:
        u32 idx;
    };
}
