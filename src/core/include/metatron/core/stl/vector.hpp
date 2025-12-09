#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/stack.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/vector.hpp>
#include <vector>
#include <functional>
#include <typeindex>
#include <mutex>

namespace mtt::stl {
    template<typename T>
    struct vector;

    template<>
    struct vector<byte> final {
        auto static constexpr block_size = 1 << 6;
        auto static constexpr max_idx = 1 << 20;
        u32 bytelen = 0;
        std::function<void()> destroier;
        ~vector() noexcept {
            if (destroier) destroier();
            for (auto* block: blocks) std::free(block);
        }

        template<typename T>
        auto init() noexcept -> void {
            bytelen = sizeof(T);
            if constexpr (!std::is_trivially_destructible_v<T>) destroier = [this] {
                for (auto i = 0; i < blocks.size(); ++i) {
                    auto size = i == blocks.size() - 1
                    ? length % block_size : block_size;
                    std::destroy_n(mut<T>(blocks[i]), size);
                }
            };
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            auto idx = length.fetch_add(1);
            auto block = idx / block_size;
            auto start = block * block_size;
            auto local_idx = idx % block_size;
            if (idx >= max_idx) stl::abort("vector overflow");

            while (fetched.load(std::memory_order::acquire) < start);
            if (local_idx == 0) {
                blocks.push_back(mut<byte>(std::malloc(bytelen * block_size)));
                allocated.fetch_add(1, std::memory_order::release);
            } else while (allocated.load(std::memory_order::acquire) <= block);

            auto ptr = mut<T>(blocks[block] + local_idx * bytelen);
            fetched.fetch_add(1, std::memory_order::release);
            std::construct_at(ptr, std::forward<Args>(args)...);
            return idx;
        }
        template<typename T>
        requires std::is_constructible_v<std::decay_t<T>, T>
        auto push_back(T&& x) noexcept -> u32 {
            return emplace_back<std::decay_t<T>>(std::forward<T>(x));
        }

        auto size() const noexcept -> usize { return length; }
        auto operator[](u32 i) noexcept -> mut<byte> {
            return blocks[i / block_size] + bytelen * (i % block_size);
        }
        auto operator[](u32 i) const noexcept -> view<byte> {
            return blocks[i / block_size] + bytelen * (i % block_size);
        }

    private:
        std::vector<mut<byte>> blocks;
        std::atomic<u32> length = 0;
        std::atomic<u32> allocated = 0;
        std::atomic<u32> fetched = 0;
    };

    template<>
    struct vector<void> final: singleton<vector<void>> {
        auto constexpr static max_idx = 256;
        std::array<stl::vector<byte>, max_idx> storage;

        template<typename T>
        auto push() noexcept -> u32 {
            auto idx = length;
            if (idx >= max_idx) stl::abort("pointer vectors overflow");
            ++length;
            storage[idx].init<T>();
            return idx;
        }

    private:
        u32 length = 0;
    };

    template<typename T>
    struct vector final: singleton<vector<T>> {
        vector() noexcept {
            tid = vector<void>::instance().push<T>();
            get().template init<T>();
        }

        template<typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            auto idx = get().template emplace_back<T>(std::forward<Args>(args)...);
            return (tid << 24) | idx;
        }
        auto push_back(rref<T> x) noexcept -> u32 { return emplace_back(std::move(x)); }
        auto push_back(cref<T> x) noexcept -> u32 { return emplace_back(x); }

        auto operator[](u32 i) noexcept -> mut<T> { return mut<T>(get()[i & 0xfffff]); }
        auto operator[](u32 i) const noexcept -> view<T> { return view<T>(get()[i & 0xfffff]); }
        auto size() const noexcept -> usize { return get().size(); }

    private:
        auto get() noexcept -> ref<vector<byte>> {
            return vector<void>::instance().storage[tid];
        }

        auto get() const noexcept -> cref<vector<byte>> {
            return vector<void>::instance().storage[tid];
        }

        u32 tid;
        obj<std::mutex> mutex;
    };

    template<pro::facade F>
    struct vector<F> final: singleton<vector<F>> {
        auto constexpr static max_idx = 1 << 4;

        template<typename T>
        requires poliable<F, T>
        auto emplace_type() noexcept -> void {
            if (map.contains(typeid(T))) return;
            if (sid.size() >= max_idx) stl::abort("facade vector overflow");
            sid.push_back(vector<void>::instance().push<T>());
            map[typeid(T)] = sid.size() - 1;
            length.push_back(sizeof(T));
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
            auto tid = map[typeid(T)];
            auto& stroage = get(sid[tid]);
            auto idx = stroage.template emplace_back<T>(std::forward<Args>(args)...);
            return sid[tid] << 24 | tid << 20 | idx;
        }

        template<typename T>
        requires std::is_constructible_v<std::decay_t<T>, T>
        auto push_back(T&& x) noexcept -> u32 {
            return emplace_back<std::decay_t<T>>(std::forward<T>(x));
        }

        auto operator[](u32 idx) noexcept -> mut<F> {
            auto [s, t, i] = split(idx);
            auto ptr = get(s)[i];
            return reinterpreter[t](ptr);
        }

        auto operator[](u32 idx) const noexcept -> view<F> {
            auto [s, t, i] = split(idx);
            auto ptr = get(s)[i];
            return reinterpreter[t](ptr);
        }

        auto operator()(u32 idx) const noexcept -> obj<F>
        requires(F::copyability != pro::constraint_level::none) {
            auto [s, t, i] = split(idx);
            auto ptr = get(s)[i];
            return copier.at(t)(ptr);
        }

        template<typename T>
        auto is(u32 idx) const noexcept -> bool {
            auto [s, t, i] = split(idx);
            return true
            && map.contains(typeid(T))
            && t == map.at(typeid(T))
            && i < get(s).size();
        }

        template<typename T>
        auto size() const noexcept -> usize { return get(sid[map.at(typeid(T))]).size(); }

    private:
        auto split(u32 idx) noexcept -> uv3 {
            return {
                idx >> 24,
                idx >> 20 & 0xf,
                idx & 0xfffff,
            };
        }

        auto get(u32 tid) noexcept -> ref<vector<byte>> {
            return vector<void>::instance().storage[tid];
        }

        auto get(u32 tid) const noexcept -> cref<vector<byte>> {
            return vector<void>::instance().storage[tid];
        }

        std::unordered_map<std::type_index, u32> map;
        std::vector<u32> sid;
        std::vector<u32> length;
        std::vector<std::function<mut<F>(view<byte> ptr)>> reinterpreter;
        std::vector<std::function<obj<F>(view<byte> ptr)>> copier;
    };
}

namespace mtt {
    template<typename T>
    struct tag final {
        using vec = stl::vector<T>;

        tag(): idx(math::maxv<u32>) {};
        tag(u32 idx): idx(idx) {}

        auto storage() noexcept -> u32 { return idx >> 24; }
        auto type() noexcept -> u32 { return idx >> 20 & 0xf; };
        auto index() noexcept -> u32 { return idx & 0xffff; };
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
