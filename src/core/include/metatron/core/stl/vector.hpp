#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/array.hpp>
#include <metatron/core/stl/string.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/vector.hpp>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <atomic>

namespace mtt::stl {
    template<typename... Ts>
    struct vector;

    template<>
    struct vector<byte> final {
        auto static constexpr block_size = 1 << 6;
        auto static constexpr max_idx = 1 << 20;
        u32 bytelen = 0;
        ~vector() noexcept { release(); }

        template<typename T>
        auto destroy() noexcept -> void {
            for (auto i = 0; i < blocks.size(); ++i) {
                auto size = i == blocks.size() - 1
                ? length % block_size : block_size;
                std::destroy_n(mut<T>(blocks[i]), size);
            }
        }
        auto (vector::*destroier)() -> void;

        template<typename T>
        auto init() noexcept -> void {
            bytelen = sizeof(T);
            if constexpr (!std::is_trivially_destructible_v<T>)
                destroier = &vector<byte>::destroy<T>;
        }

        auto pack() noexcept -> std::vector<byte> const& {
            buffer.resize(length * bytelen);
            for (auto i = 0; i < blocks.size(); ++i) {
                auto size = (i < blocks.size() - 1 ? block_size : length % block_size);
                auto offset = buffer.data() + i * block_size * bytelen;
                std::memcpy(offset, blocks[i], size * bytelen);
            }
            blocks.clear();
            return buffer;
        }

        auto release() noexcept -> void {
            if (destroier) (this->*destroier)();
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
            if (blocks.empty()) [[likely]]
                return mut<byte>(buffer.data()) + bytelen * i;
            else
                return blocks[i / block_size] + bytelen * (i % block_size);
        }

        std::vector<mut<byte>> blocks;
        std::vector<byte> buffer;
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

    template<typename... Ts>
    requires (sizeof...(Ts) >= 1)
    struct vector<Ts...> final: singleton<vector<Ts...>> {
        using ts = stl::array<Ts...>;
        using F = ts::template type<0>;
        vector() noexcept {
            base_storage = vector<void>::instance().push<F>(); raw<F>().template init<F>();
            (((void)((ts::template index<Ts> > 0) ? (
                vector<void>::instance().push<Ts>(), raw<Ts>().template init<Ts>()
            , 0) : 0)), ...);
        }

        template<typename T = F, typename... Args>
        requires ts::template contains<T> && std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            auto idx = raw<T>().template emplace_back<T>(std::forward<Args>(args)...);
            return (storage<T>() << 24) | (ts::template index<T> << 20) | idx;
        }

        template<typename T = F, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace(std::string_view path, Args&&... args) noexcept -> u32 {
            auto idx = raw<T>().template emplace<T>(path, std::forward<Args>(args)...);
            return (storage<T>() << 24) | (ts::template index<T> << 20) | idx;
        }

        auto entity(std::string_view path) noexcept -> u32 {
            auto idx = u32{};
            auto _ = ((contains<Ts>(path) ? (idx = entity<Ts>(path), true) : false) || ...);
            return idx;
        }

        template<typename T = F>
        auto push_back(rref<T> x) noexcept -> u32 { return emplace_back<T>(std::move(x)); }
        template<typename T = F>
        auto push_back(cref<T> x) noexcept -> u32 { return emplace_back<T>(x); }
        template<typename T = F>
        auto push(std::string_view path, rref<T> x) noexcept -> u32 { return emplace<T>(path, std::move(x)); }
        template<typename T = F>
        auto push(std::string_view path, cref<T> x) noexcept -> u32 { return emplace<T>(path, x); }

        template<typename T = F>
        auto operator[](u32 i) noexcept -> ref<T> { return *mut<T>(raw<T>()[i & 0xfffff]); }
        template<typename T = F>
        auto operator[](u32 i) const noexcept -> cref<T> { return *view<T>(raw<T>()[i & 0xfffff]); }
        template<typename T = F>
        auto get(u32 i) noexcept -> mut<T> { return mut<T>(raw<T>()[i & 0xfffff]); }
        template<typename T = F>
        auto get(u32 i) const noexcept -> view<T> { return view<T>(raw<T>()[i & 0xfffff]); }
        template<typename T = F>
        auto path(u32 i) const noexcept -> std::string_view { return raw<T>().path(i & 0xfffff); }
        template<typename T = F>
        auto entity(std::string_view path) const noexcept -> u32 {
            return (storage<T>() << 24) | (ts::template index<T> << 20) | raw<T>().entity(path);
        }
        template<typename T = F>
        auto contains(std::string_view path) const noexcept -> bool { return raw<T>().contains(path); }
        template<typename T = F>
        auto storage() const noexcept -> u32 { return base_storage + ts::template index<T>; }
        template<typename T = F>
        auto size() const noexcept -> usize { return raw<T>().size(); }
        template<typename T = F>
        auto keys() const noexcept { return raw<T>().keys(); }

        auto raw(u32 type_idx) noexcept -> ref<vector<byte>> {
            return vector<void>::instance().storage[base_storage + type_idx];
        }
        auto raw(u32 type_idx) const noexcept -> cref<vector<byte>> {
            return vector<void>::instance().storage[base_storage + type_idx];
        }

    private:
        template<typename T>
        auto raw() noexcept -> ref<vector<byte>> {
            return vector<void>::instance().storage[base_storage + ts::template index<T>];
        }
        template<typename T>
        auto raw() const noexcept -> cref<vector<byte>> {
            return vector<void>::instance().storage[base_storage + ts::template index<T>];
        }

        u32 base_storage = 0;
    };
}

namespace mtt {
    template<typename... Ts>
    struct tag final {
        using ts = stl::array<Ts...>;
        using vs = stl::vector<Ts...>;
        using F = ts::template type<0>;

        tag(): idx(math::maxv<u32>) {};
        tag(u32 idx): idx(idx) {}

        auto storage() const noexcept -> u32 { return idx >> 24; }
        auto type() const noexcept -> u32 { return (idx >> 20) & 0xf; }
        auto index() const noexcept -> u32 { return idx & 0xfffff; }
        template<typename T = F>
        auto data() noexcept -> mut<T> { return vs::instance().template get<T>(idx); }
        template<typename T = F>
        auto data() const noexcept -> view<T> { return vs::instance().template get<T>(idx); }
        template<typename T>
        auto is() const noexcept -> bool { return vs::instance().template is<T>(idx); }

        template<typename T = F>
        auto operator->() noexcept -> mut<T> { return data(); }
        template<typename T = F>
        auto operator->() const noexcept -> view<T> { return data(); }
        template<typename T = F>
        auto operator*() noexcept -> ref<T> { return *data(); }
        template<typename T = F>
        auto operator*() const noexcept -> cref<T> { return *data(); }

        operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return idx != math::maxv<u32>; }

    private:
        u32 idx;
    };

}
