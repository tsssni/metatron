#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/stack.hpp>
#include <metatron/core/math/constant.hpp>
#include <vector>
#include <functional>
#include <typeindex>
#include <mutex>

namespace mtt::stl {
    template<typename T>
    struct vector final: singleton<vector<T>> {
        vector() noexcept: mutex(make_obj<std::mutex>()) {}

        auto pack() noexcept -> void {
            buf.ptr = storage.data();
            buf.bytelen = storage.size() * sizeof(T);
            stl::stack::instance().push(&buf, [this] {
                storage.clear();
            });
        }

        template<typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            storage.emplace_back(std::forward<Args>(args)...);
            return storage.size() - 1;
        }
        auto push_back(rref<T> x) noexcept -> u32 { return emplace_back(std::move(x)); }
        auto push_back(cref<T> x) noexcept -> u32 { return emplace_back(x); }
        auto lock() const noexcept { return std::unique_lock{*mutex };}

        auto operator[](u32 i) noexcept -> mut<T> { return &storage[i]; }
        auto operator[](u32 i) const noexcept -> view<T> { return &storage[i]; }
        auto addr() const noexcept -> uptr { return uptr(buf.ptr); }
        auto data() const noexcept -> mut<T> { return storage.data(); }
        auto size() const noexcept -> usize { return storage.size(); }
        auto empty() const noexcept -> usize { return storage.empty(); }
        auto capacity() const noexcept -> usize { return storage.capacity(); }
        auto reserve(usize n) noexcept -> void { storage.reserve(n); }

    private:
        stl::buf buf;
        std::vector<T> storage;
        obj<std::mutex> mutex;
    };

    template<pro::facade F>
    struct vector<F> final: singleton<vector<F>> {
        vector() noexcept {gutex = make_obj<std::mutex>();}
        ~vector() noexcept {
            for (auto i = 0; i < destroier.size(); ++i)
                destroier[i](storage[i]);
        }

        auto pack() noexcept -> void {
            for (auto i = 0; i < i32(buf.size()); ++i) {
                buf[i].ptr = storage[i].data();
                buf[i].bytelen = storage[i].size();
                stl::stack::instance().push(&buf[i], [this, i] {
                    storage[i].clear();
                });
            }
        }

        template<typename T>
        requires poliable<F, T>
        auto emplace_type() noexcept -> void {
            auto&& tid = typeid(T);
            auto mid = map.size();
            if (map.contains(tid)) return;

            map[tid] = mid;
            buf.push_back({});
            storage.push_back({});
            length.push_back(sizeof(T));
            mutex.push_back(make_obj<std::mutex>());

            destroier.push_back([](ref<std::vector<byte>> vec) {
                std::destroy_n((mut<T>)vec.data(), vec.size() / sizeof(T));
            });
            reinterpreter.push_back([](view<byte> ptr) {
                return make_mut<F>(*(mut<T>)ptr);
            });
            if constexpr (F::copyability != pro::constraint_level::none)
                copier.push_back([](view<byte> ptr) {
                    auto x = *(mut<T>)ptr; return make_obj<F, T>(std::move(x));
                });
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            auto idx = map[typeid(T)];
            auto size = storage[idx].size();
            storage[idx].resize(size + sizeof(T));
            auto ptr = mut<T>(storage[idx].data() + size);
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

        auto operator()(u32 i) const noexcept -> obj<F>
        requires(F::copyability != pro::constraint_level::none) {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            auto ptr = storage[t].data() + length[t] * idx;
            return copier.at(t)(ptr);
        }

        template<typename T>
        auto is(u32 i) const noexcept -> bool {
            auto t = (i >> 24) & 0xff;
            auto idx = (i & 0xffffff);
            return true
            && map.contains(typeid(T))
            && t == map.at(typeid(T))
            && idx < storage[t].size();
        }

        template<typename T>
        auto addr() const noexcept -> uptr { return uptr(buf[map.at(typeid(T))].ptr); }
        template<typename T>
        auto data() const noexcept -> mut<T> { return (mut<T>)storage[map.at(typeid(T))].data(); }
        template<typename T>
        auto size() const noexcept -> usize { return storage[map.at(typeid(T))].size() / sizeof(T); }
        template<typename T>
        auto empty() const noexcept -> usize { return storage[map.at(typeid(T))].empty(); }
        template<typename T>
        auto capacity() const noexcept -> usize { return storage[map.at(typeid(T))].capacity() / sizeof(T); }
        template<typename T>
        auto reserve(usize n) noexcept -> void { storage[map.at(typeid(T))].reserve(n * sizeof(T)); }

    private:
        std::unordered_map<std::type_index, u32> map;
        std::vector<stl::buf> buf;
        std::vector<std::vector<byte>> storage;
        std::vector<u32> length;
        std::vector<std::function<auto (ref<std::vector<byte>> vec) -> void>> destroier;
        std::vector<std::function<auto (view<byte> ptr) -> mut<F>>> reinterpreter;
        std::vector<std::function<auto (view<byte> ptr) -> obj<F>>> copier;
        std::vector<obj<std::mutex>> mutex;
        obj<std::mutex> gutex;
    };
}

namespace mtt {
    template<typename T>
    struct tag final {
        using vec = stl::vector<T>;

        tag(): idx(math::maxv<u32>) {};
        tag(u32 idx): idx(idx) {}

        auto type() noexcept -> u32 { return (idx >> 24) & 0xff; };
        auto index() noexcept -> u32 { return idx & 0xffffff; };
        auto data() noexcept -> mut<T> { return vec::instance()[idx]; }
        auto data() const noexcept -> view<T> { return vec::instance()[idx]; }

        auto operator->() noexcept -> mut<T> { return data(); }
        auto operator->() const noexcept -> view<T> { return data(); }
        auto operator*() noexcept -> ref<T> requires(!pro::facade<T>) { return *data(); }
        auto operator*() const noexcept -> cref<T> requires(!pro::facade<T>) { return *data(); }
        auto operator*() const noexcept -> obj<T>
        requires(pro::facade<T> && T::copyability != pro::constraint_level::none) {
            return vec::instance()(idx);
         }

        explicit operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return idx != math::maxv<u32>; }

    private:
        u32 idx;
    };
}
