#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/string.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/vector.hpp>
#include <vector>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <cstring>

namespace mtt::stl {
    template<typename T>
    struct vector;

    template<>
    struct vector<byte> final {
        auto static constexpr block_size = 1 << 6;
        auto static constexpr max_idx = 1 << 20;
        u32 bytelen = 0;
        std::function<void()> destroier;
        ~vector() noexcept { release(); }

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

        auto pack() noexcept -> std::vector<byte> {
            auto buffer = std::vector<byte>{};
            buffer.resize(length * bytelen);
            for (auto i = 0; i < blocks.size(); ++i) {
                auto size = (i < blocks.size() - 1 ? block_size : length % block_size);
                auto offset = buffer.data() + i * block_size * bytelen;
                std::memcpy(offset, blocks[i], size * bytelen);
            }
            return buffer;
        }

        auto release() noexcept -> void {
            if (destroier) destroier();
            for (auto* block: blocks) std::free(block);
            destroier = nullptr;
            blocks.clear();
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            auto [ptr, idx] = spin();
            fetched.fetch_add(1, std::memory_order::release);
            std::construct_at(mut<T>(ptr), std::forward<Args>(args)...);
            return idx;
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace(std::string_view path, Args&&... args) noexcept -> u32 {
            auto str = std::string{path};
            auto [ptr, idx] = spin();
            pathes[idx] = str;
            fetched.fetch_add(1, std::memory_order::release);
            std::construct_at(mut<T>(ptr), std::forward<Args>(args)...);
            while (flag.test_and_set(std::memory_order::acquire));
            entities[str] = idx;
            flag.clear(std::memory_order::release);
            return idx;
        }

        template<typename T>
        requires std::is_constructible_v<std::decay_t<T>, T>
        auto push_back(T&& x) noexcept -> u32 {
            return emplace_back<std::decay_t<T>>(std::forward<T>(x));
        }

        template<typename T>
        requires std::is_constructible_v<std::decay_t<T>, T>
        auto push(std::string_view path, T&& x) noexcept -> u32 {
            return emplace<std::decay_t<T>>(path, std::forward<T>(x));
        }

        auto operator[](u32 i) noexcept -> mut<byte> { return at(i); }
        auto operator[](u32 i) const noexcept -> view<byte> { return at(i); }
        auto path(u32 i) const noexcept -> std::string_view { return pathes[i]; }
        auto entity(std::string_view path) const noexcept -> u32 { return at(path); }
        auto contains(std::string_view path) const noexcept -> bool { return entities.contains(path); }
        auto size() const noexcept -> usize { return length; }
        auto keys() const noexcept { return entities | std::views::keys; }

    private:
        auto spin() noexcept -> std::tuple<mut<byte>, u32> {
            auto idx = length.fetch_add(1);
            auto block = idx / block_size;
            auto start = block * block_size;
            auto local_idx = idx % block_size;
            if (idx >= max_idx) stl::abort("vector overflow");

            while (fetched.load(std::memory_order::acquire) < start);
            if (local_idx == 0) {
                blocks.push_back(mut<byte>(std::malloc(bytelen * block_size)));
                pathes.resize(start + block_size);
                allocated.fetch_add(1, std::memory_order::release);
            } else while (allocated.load(std::memory_order::acquire) <= block);
            
            auto ptr = blocks[block] + local_idx * bytelen;
            return {ptr, idx};
        }

        auto at(std::string_view path) const noexcept -> u32 {
            auto iter = entities.find(path);
            if (iter == entities.end()) stl::abort("empty entity");
            return iter->second;
        }

        auto at(u32 i) const noexcept -> mut<byte> {
            return blocks[i / block_size] + bytelen * (i % block_size);
        }

        std::vector<mut<byte>> blocks;
        std::vector<std::string> pathes;
        table<u32> entities;

        std::atomic<u32> length = 0;
        std::atomic<u32> allocated = 0;
        std::atomic<u32> fetched = 0;
        std::atomic_flag flag = false;
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
        auto size() noexcept -> u32 { return length; }

    private:
        u32 length = 0;
    };

    template<typename T>
    struct vector final: singleton<vector<T>> {
        vector() noexcept {
            tid = vector<void>::instance().push<T>();
            raw().template init<T>();
        }

        template<typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            auto idx = raw().template emplace_back<T>(std::forward<Args>(args)...);
            return (tid << 24) | idx;
        }

        template<typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace(std::string_view path, Args&&... args) noexcept -> u32 {
            auto idx = raw().template emplace<T>(path, std::forward<Args>(args)...);
            return (tid << 24) | idx;
        }

        auto push_back(rref<T> x) noexcept -> u32 { return emplace_back(std::move(x)); }
        auto push_back(cref<T> x) noexcept -> u32 { return emplace_back(x); }
        auto push(std::string_view path, rref<T> x) noexcept -> u32 {
            return emplace(path, std::move(x));
        }
        auto push(std::string_view path, cref<T> x) noexcept -> u32 {
            return emplace(path, x);
        }

        auto operator[](u32 i) noexcept -> mut<T> { return mut<T>(raw()[i & 0xfffff]); }
        auto operator[](u32 i) const noexcept -> view<T> { return view<T>(raw()[i & 0xfffff]); }
        auto path(u32 i) const noexcept -> std::string_view { return raw().path(i & 0xfffff); }
        auto entity(std::string_view path) const noexcept -> u32 { return (tid << 24) | raw().entity(path); }
        auto contains(std::string_view path) const noexcept -> bool { return raw().contains(path); }
        auto index() const noexcept -> u32 { return tid; }
        auto size() const noexcept -> usize { return raw().size(); }
        auto keys() const noexcept { return raw().keys(); }

    private:
        auto raw() noexcept -> ref<vector<byte>> {
            return vector<void>::instance().storage[tid];
        }

        auto raw() const noexcept -> cref<vector<byte>> {
            return vector<void>::instance().storage[tid];
        }

        u32 tid;
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
            auto& stroage = raw(sid[tid]);
            auto idx = stroage.template emplace_back<T>(std::forward<Args>(args)...);
            return sid[tid] << 24 | tid << 20 | idx;
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace(std::string_view path, Args&&... args) noexcept -> u32 {
            auto t = map[typeid(T)];
            auto s = sid[t];
            auto& stroage = raw(s);
            auto i = stroage.template emplace<T>(path, std::forward<Args>(args)...);
            while (flag.test_and_set(std::memory_order::acquire));
            slot[std::string{path}] = t;
            if (slot.size() % vector<byte>::block_size == 0)
                slot.reserve(slot.size() + vector<byte>::block_size);
            flag.clear(std::memory_order::release);
            return s << 24 | t << 20 | i;
        }

        template<typename T>
        requires std::is_constructible_v<std::decay_t<T>, T>
        auto push_back(T&& x) noexcept -> u32 {
            return emplace_back<std::decay_t<T>>(std::forward<T>(x));
        }

        template<typename T>
        requires std::is_constructible_v<std::decay_t<T>, T>
        auto push(std::string_view path, T&& x) noexcept -> u32 {
            return emplace<std::decay_t<T>>(path, std::forward<T>(x));
        }

        auto operator[](u32 idx) noexcept -> mut<F> {
            auto [s, t, i] = split(idx);
            auto ptr = raw(s)[i];
            return reinterpreter[t](ptr);
        }

        auto operator[](u32 idx) const noexcept -> view<F> {
            auto [s, t, i] = split(idx);
            auto ptr = raw(s)[i];
            return reinterpreter[t](ptr);
        }

        auto operator()(u32 idx) const noexcept -> obj<F>
        requires(F::copyability != pro::constraint_level::none) {
            auto [s, t, i] = split(idx);
            auto ptr = raw(s)[i];
            return copier.at(t)(ptr);
        }

        template<typename T>
        auto get(u32 idx) const noexcept -> view<T> {
            auto t = map.at(typeid(T));
            auto s = sid[t];
            auto ptr = raw(s)[idx];
            return view<T>(ptr);
        }

        auto path(u32 idx) const noexcept -> std::string_view {
            auto [s, t, i] = split(idx);
            return raw(s).path(i);
        }

        auto entity(std::string_view path) const noexcept -> u32 {
            auto iter = slot.find(path);
            if (iter == slot.end()) stl::abort("empty facade entity");
            auto t = iter->second;
            auto s = sid[t];
            auto i = raw(s).entity(path);
            return s << 24 | t << 20 | i;
        }

        template<typename T>
        auto is(u32 idx) const noexcept -> bool {
            auto [s, t, i] = split(idx);
            return true
            && map.contains(typeid(T))
            && t == map.at(typeid(T))
            && i < raw(s).size();
        }

        auto contains(std::string_view path) const noexcept -> bool {
            return slot.contains(path);
        }

        template<typename T>
        auto index() const noexcept -> u32 { return sid[map.at(typeid(T))]; }
        template<typename T>
        auto size() const noexcept -> usize { return raw(sid[map.at(typeid(T))]).size(); }
        auto keys() const noexcept { return slot | std::views::keys; }

    private:
        auto split(u32 idx) const noexcept -> uv3 {
            return {
                idx >> 24,
                idx >> 20 & 0xf,
                idx & 0xfffff,
            };
        }

        auto raw(u32 tid) noexcept -> ref<vector<byte>> {
            return vector<void>::instance().storage[tid];
        }

        auto raw(u32 tid) const noexcept -> cref<vector<byte>> {
            return vector<void>::instance().storage[tid];
        }

        std::unordered_map<std::type_index, u32> map;
        std::unordered_map<std::string, u32,
        stl::hash<std::string>, std::equal_to<>> slot;
        std::atomic_flag flag = false;

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
        template<typename U>
        auto is() const noexcept -> bool { return vec::instance().template is<U>(idx); }

        auto operator->() noexcept -> mut<T> { return data(); }
        auto operator->() const noexcept -> view<T> { return data(); }
        auto operator*() noexcept -> ref<T> requires(!pro::facade<T>) { return *data(); }
        auto operator*() const noexcept -> cref<T> requires(!pro::facade<T>) { return *data(); }
        auto operator*() const noexcept -> obj<T>
        requires(pro::facade<T> && T::copyability != pro::constraint_level::none) {
            return vec::instance()(idx);
        }

        operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return idx != math::maxv<u32>; }

    private:
        u32 idx;
    };

    template<typename T>
    auto entity(std::string_view path) noexcept -> tag<T> {
        return stl::vector<T>::instance().entity(path);
    }

    template<typename T>
    auto path(tag<T> idx) noexcept -> std::string_view {
        return stl::vector<T>::instance().path(idx);
    }
}
