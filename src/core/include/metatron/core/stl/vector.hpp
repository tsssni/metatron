#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/math/constant.hpp>
#include <vector>
#include <functional>
#include <typeindex>
#include <mutex>

namespace mtt::stl {
    template<typename T>
    struct vector final: singleton<vector<T>> {
        vector() noexcept: mutex(make_poly<std::mutex>()) {}
        vector(vector&&) noexcept = default;

        template<typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            storage.emplace_back(std::forward<Args>(args)...);
            return storage.size() - 1;
        }
        auto push_back(T&& x) noexcept -> u32 {return emplace_back(std::move(x));}
        auto push_back(T const& x) noexcept -> u32 {return emplace_back(x);}
        auto lock() const noexcept -> std::unique_lock<std::mutex> {return std::unique_lock{*mutex};}

        auto operator[](u32 i) noexcept -> mut<T> {return &storage[i];}
        auto operator[](u32 i) const noexcept -> view<T> {return &storage[i];}
        auto data() const noexcept -> T* {return storage.data();}
        auto size() const noexcept -> usize {return storage.size();}
        auto capacity() const noexcept -> usize {return storage.capacity();}
        auto reserve(usize n) noexcept -> void {storage.reserve(n);}

    private:
        std::vector<T> storage;
        poly<std::mutex> mutex;
    };

    template<pro::facade F>
    struct vector<F> final: singleton<vector<F>> {
        vector() noexcept {gutex = make_poly<std::mutex>();}
        ~vector() noexcept {
            for (auto i = 0; i < destroier.size(); i++) {
                auto& d = destroier[i];
                for (auto j = 0; j < storage[i].size() / length[i]; j++)
                    d(storage[i].data() + j * length[i]);
            }
        }

        template<typename T>
        requires poliable<F, T>
        auto emplace_type() noexcept -> void {
            auto idx = map.size();
            map[typeid(T)] = idx;
            storage.push_back({});
            length.push_back(sizeof(T));
            mutex.push_back(make_poly<std::mutex>());
            reinterpreter.push_back([](byte const* ptr) {return make_view<F>(*(T*)ptr);});
            destroier.push_back([](byte* ptr) {std::destroy_at((T*)ptr);});
            if constexpr (F::copyability != pro::constraint_level::none)
                copier.push_back([](byte const* ptr) {
                    auto x = *(T*)ptr; return make_poly<F, T>(std::move(x));
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
            return emplace_back<std::decay_t<T>>(std::forward<T>(x));
        }

        template<typename T>
        auto lock() const noexcept -> std::unique_lock<std::mutex> {
            return std::unique_lock{*mutex[map.at(typeid(T))]};
        }

        auto lock() const noexcept -> std::unique_lock<std::mutex> {
            return std::unique_lock{*gutex};
        }

        auto operator[](u32 i) noexcept -> mut<F> {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            auto ptr = storage[t].data() + length[t] * idx;
            return reinterpreter[t](ptr);
        }

        auto operator[](u32 i) const noexcept -> view<F> {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            auto ptr = storage[t].data() + length[t] * idx;
            return reinterpreter.at(t)(ptr);
        }

        auto operator()(u32 i) const noexcept -> poly<F>
        requires(F::copyability != pro::constraint_level::none) {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            auto ptr = storage[t].data() + length[t] * idx;
            return copier.at(t)(ptr);
        }

        template<typename T>
        auto data() const noexcept -> T* {return (T*)storage[map.at(typeid(T))].data();}
        template<typename T>
        auto size() const noexcept -> usize {return storage[map.at(typeid(T))].size() / sizeof(T);}
        template<typename T>
        auto capacity() const noexcept -> usize {return storage[map.at(typeid(T))].capacity() / sizeof(T);}
        template<typename T>
        auto reserve(usize n) noexcept -> void {storage[map.at(typeid(T))].reserve(n * sizeof(T));}

    private:
        std::vector<std::vector<byte>> storage;
        std::vector<u32> length;
        std::vector<std::function<auto (byte const* ptr) -> mut<F>>> reinterpreter;
        std::vector<std::function<auto (byte const* ptr) -> poly<F>>> copier;
        std::vector<std::function<auto (byte* ptr) -> void>> destroier;
        std::vector<poly<std::mutex>> mutex;
        poly<std::mutex> gutex;
        std::unordered_map<std::type_index, u32> map;
    };
}

namespace mtt {
    template<typename T>
    struct proxy final {
        using vec = stl::vector<T>;

        proxy(): idx(math::maxv<u32>) {};
        proxy(u32 idx): idx(idx) {}

        auto data() -> mut<T> {return vec::instance()[idx];}
        auto data() const -> view<T> {return vec::instance()[idx];}

        auto operator->() -> mut<T> {return data();}
        auto operator->() const -> view<T> {return data();}
        auto operator*() -> T& requires(!pro::facade<T>) {return *data();}
        auto operator*() const -> T const& requires(!pro::facade<T>) {return *data();}
        auto operator*() const -> poly<T>
        requires(pro::facade<T> && T::copyability != pro::constraint_level::none) {
            return vec::instance()(idx);
        }

        explicit operator u32() const {return idx;}
        operator bool() const {return idx != math::maxv<u32>;}

    private:
        u32 idx;
    };
}
