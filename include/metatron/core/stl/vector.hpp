#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/array.hpp>
#include <metatron/core/stl/string.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/math/vector.hpp>
#include <vector>
#include <cstring>
#include <atomic>

namespace mtt::stl {
    template<typename... Ts>
    struct vector;

    template<>
    struct vector<byte> final {
        auto static constexpr max_idx = 1 << 20;
        auto static constexpr slot_mask = max_idx - 1;
        auto static constexpr block_bit = 10;
        auto static constexpr block_size = 1 << block_bit;
        auto static constexpr block_mask = block_size - 1;
        auto static constexpr block_count = max_idx / block_size;
        u32 bytelen = 0;
        ~vector() noexcept { release(); }

        template<typename T>
        auto init() noexcept -> void {
            bytelen = sizeof(T);
            destroier = &vector<byte>::destroy<T>;
        }

        auto pack() noexcept -> std::vector<byte> const& {
            auto len = (i32)length.load(std::memory_order::relaxed);
            buffer.resize(len * bytelen);
            for (auto i = 0; i < (len + block_size - 1) >> block_bit; ++i) {
                auto size = math::min(len - (i << block_bit), block_size);
                auto offset = buffer.data() + i * block_size * bytelen;
                auto block = blocks[i].load(std::memory_order::relaxed);
                std::memcpy(offset, block, size * bytelen);
                std::free(block);
                blocks[i].store(nullptr, std::memory_order::relaxed);
            }
            return buffer;
        }

        auto release() noexcept -> void {
            if (destroier) (this->*destroier)();
            auto rel = []<typename... Args>(Args&&... ptrs) {
                auto r = [](auto&& ptrs) {
                    for (auto& ptr: ptrs) std::free(ptr.load(std::memory_order::relaxed));
                    return true;
                };
                (r(ptrs) && ...);
            };
            rel(blocks, pathes, slots);
            destroier = nullptr;
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace_back(Args&&... args) noexcept -> u32 {
            auto [ptr, idx] = alloc();
            std::construct_at(mut<T>(ptr), std::forward<Args>(args)...);
            return idx;
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto emplace(std::string_view path, Args&&... args) noexcept -> u32 {
            auto [ptr, idx] = alloc();
            std::construct_at(mut<T>(ptr), std::forward<Args>(args)...);
            *(exchange<true>(pathes, idx) + (idx & block_mask)) = path;

            for (auto p = (u32)(std::hash<std::string_view>{}(path) & slot_mask);;p = (p + 1) & slot_mask) {
                auto& slot = *(exchange<true>(slots, p, sizeof(u32), math::maxv<u32>) + (p & block_mask));
                auto s = slot.load(std::memory_order::acquire);
                if (s == math::maxv<u32> && slot.compare_exchange_strong(s, idx, std::memory_order::release, std::memory_order::relaxed)) break;
                // concurrent emplace with same path is not allowed
            }
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

        auto keys() const noexcept {
            return std::views::iota(0u, u32(size()))
            | std::views::transform([this](u32 i) { return find<true>(i); })
            | std::views::filter([](auto str) { return str != ""; })
            | std::ranges::to<std::vector<std::string_view>>();
        }

        auto get(u32 i) noexcept -> mut<byte> { return at(i); }
        auto get(u32 i) const noexcept -> view<byte> { return at(i); }
        auto path(u32 i) const noexcept -> std::string_view { return find<false>(i); }
        auto entity(std::string_view path) const noexcept -> u32 { return at<false>(path); }
        auto contains(std::string_view path) const noexcept -> bool { return at<true>(path) != math::maxv<u32>; }
        auto size() const noexcept -> usize { return length.load(std::memory_order::relaxed); }

    private:
        template<bool init = false, typename T, typename V = T>
        auto exchange(ref<std::array<std::atomic<T*>, block_count>> ptrs, u32 idx, u32 bytelen = sizeof(T), rref<V> val = {}) noexcept -> mut<T> {
            auto b = idx / block_size;
            auto ptr = ptrs[b].load(std::memory_order::acquire);
            if (!ptr) {
                auto alloc = (mut<T>)std::malloc(bytelen * block_size);
                if constexpr (init) std::uninitialized_fill_n(alloc, block_size, std::forward<V>(val));
                if (ptrs[b].compare_exchange_strong(ptr, alloc, std::memory_order::release, std::memory_order::acquire)) {
                    ptr = alloc;
                } else {
                    if constexpr (init) std::destroy_n(alloc, block_size);
                    std::free(alloc);
                }
            }
            return ptr;
        }

        auto alloc() noexcept -> std::tuple<mut<byte>, u32> {
            auto idx = length.fetch_add(1, std::memory_order::relaxed);
            auto i = idx & block_mask;
            if (idx >= max_idx) stl::abort("vector overflow");
            auto bp = exchange(blocks, idx, bytelen);
            return {bp + i * bytelen, idx};
        }

        template<bool silent, typename T>
        auto at(cref<std::array<std::atomic<T*>, block_count>> ptrs, u32 idx, u32 bytelen = 1) const noexcept -> mut<T> {
            auto b = ptrs[idx >> block_bit].load(std::memory_order::relaxed);
            if (!b) [[unlikely]] {
                if constexpr (!silent) stl::abort("block not allocated");
                else return nullptr;
            }
            return b + (idx & block_mask) * bytelen;
        }

        template<bool silent>
        auto at(std::string_view path) const noexcept -> u32 {
            auto abort = []{
                if constexpr (!silent) stl::abort("empty entity");
                return math::maxv<u32>;
            };
            for (auto p = (u32)(std::hash<std::string_view>{}(path) & slot_mask);;p = (p + 1) & slot_mask) {
                auto block = slots[p >> block_bit].load(std::memory_order::acquire);
                if (!block) [[unlikely]] return abort();
                auto s = block[p & block_mask].load(std::memory_order::acquire);
                if (s == math::maxv<u32>) return abort();
                if (find<silent>(s) == path) return s;
            }
        }

        auto at(u32 i) const noexcept -> mut<byte> {
            if (!buffer.empty()) [[likely]] return mut<byte>(buffer.data()) + bytelen * i;
            else return at<false>(blocks, i, bytelen);
        }

        template<bool silent>
        auto find(u32 i) const noexcept -> std::string_view {
            auto abort = []{
                if constexpr (!silent) stl::abort("path not set");
                return "";
            };
            if (i >= length.load(std::memory_order::relaxed)) [[unlikely]] return abort();
            auto ptr = at<true>(pathes, i);
            if (!ptr || *ptr == "") [[unlikely]] return abort();
            return *ptr;
        }

        template<typename T>
        auto destroy() noexcept -> void {
            auto len = (i32)length.load(std::memory_order::relaxed);
            for (auto i = 0; i < (len + block_size - 1) >> block_bit; ++i) {
                auto size = math::min(len - (i << block_bit), block_size);
                auto block = (mut<T>)blocks[i].load(std::memory_order::relaxed);
                auto path = pathes[i].load(std::memory_order::relaxed);
                if constexpr (!std::is_trivially_destructible_v<T>)
                    if (block) std::destroy_n(block, size);
                if (path) std::destroy_n(path, block_size);
            }
        }

        std::array<std::atomic<mut<byte>>, block_count> blocks;
        std::array<std::atomic<mut<std::string>>, block_count> pathes;
        std::array<std::atomic<mut<std::atomic<u32>>>, block_count> slots;
        std::vector<byte> buffer;
        std::atomic<u32> length = 0;
        auto (vector::*destroier)() -> void = nullptr;
    };

    template<>
    struct vector<void> final: inline_singleton<vector<void>> {
        auto constexpr static max_idx = 256;

        template<typename T>
        auto static push() noexcept -> u32 {
            auto idx = instance().length;
            if (idx >= max_idx) stl::abort("pointer vectors overflow");
            ++instance().length;
            instance().storage[idx].init<T>();
            return idx;
        }
        auto static raw(u32 idx) noexcept -> ref<stl::vector<byte>> { return instance().storage[idx]; }
        auto static size() noexcept -> u32 { return instance().length; }

    private:
        std::array<stl::vector<byte>, max_idx> storage;
        u32 length = 0;
    };

    template<typename... Ts>
    requires (sizeof...(Ts) >= 1)
    struct vector<Ts...> final: inline_singleton<vector<Ts...>> {
        using ts = stl::array<Ts...>;
        using F = ts::template type<0>;

        auto static init() noexcept -> void {
            base_storage = vector<void>::push<F>(); raw<F>().template init<F>();
            (((void)((ts::template index<Ts> > 0) ? (
                vector<void>::push<Ts>(), raw<Ts>().template init<Ts>()
            , 0) : 0)), ...);
        }

        template<typename T = F, typename... Args>
        requires ts::template contains<T> && std::is_constructible_v<T, Args...>
        auto static emplace_back(Args&&... args) noexcept -> u32 {
            auto idx = raw<T>().template emplace_back<T>(std::forward<Args>(args)...);
            return (storage<T>() << 24) | (ts::template index<T> << 20) | idx;
        }

        template<typename T = F, typename... Args>
        requires std::is_constructible_v<T, Args...>
        auto static emplace(std::string_view path, Args&&... args) noexcept -> u32 {
            auto idx = raw<T>().template emplace<T>(path, std::forward<Args>(args)...);
            return (storage<T>() << 24) | (ts::template index<T> << 20) | idx;
        }

        auto static entity(std::string_view path) noexcept -> u32 {
            auto idx = u32{};
            auto _ = ((contains<Ts>(path) ? (idx = entity<Ts>(path), true) : false) || ...);
            return idx;
        }

        auto static raw(u32 type_idx) noexcept -> ref<vector<byte>> { return vector<void>::raw(base_storage + type_idx); }

        template<typename T = F> auto static push_back(rref<T> x) noexcept -> u32 { return emplace_back<T>(std::move(x)); }
        template<typename T = F> auto static push_back(cref<T> x) noexcept -> u32 { return emplace_back<T>(x); }
        template<typename T = F> auto static push(std::string_view path, rref<T> x) noexcept -> u32 { return emplace<T>(path, std::move(x)); }
        template<typename T = F> auto static push(std::string_view path, cref<T> x) noexcept -> u32 { return emplace<T>(path, x); }

        template<typename T = F> auto static get(u32 i) noexcept -> mut<T> { return mut<T>(raw<T>().get(i & 0xfffff)); }
        template<typename T = F> auto static path(u32 i) noexcept -> std::string_view { return raw<T>().path(i & 0xfffff); }
        template<typename T = F> auto static entity(std::string_view path) noexcept -> u32 { return (storage<T>() << 24) | (ts::template index<T> << 20) | raw<T>().entity(path); }
        template<typename T = F> auto static contains(std::string_view path) noexcept -> bool { return raw<T>().contains(path); }
        template<typename T = F> auto static storage() noexcept -> u32 { return base_storage + ts::template index<T>; }
        template<typename T = F> auto static size() noexcept -> usize { return raw<T>().size(); }
        template<typename T = F> auto static keys() noexcept { return raw<T>().keys(); }

    private:
        template<typename T> auto static raw() noexcept -> ref<vector<byte>> { return vector<void>::raw(base_storage + ts::template index<T>); }
        u32 inline static base_storage = 0;
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
        template<typename T = F> auto data() noexcept -> mut<T> { return vs::template get<T>(idx); }
        template<typename T = F> auto data() const noexcept -> view<T> { return vs::template get<T>(idx); }
        template<typename T> auto is() const noexcept -> bool { return vs::template is<T>(idx); }

        template<typename T = F> auto operator->() noexcept -> mut<T> { return data(); }
        template<typename T = F> auto operator->() const noexcept -> view<T> { return data(); }
        template<typename T = F> auto operator*() noexcept -> ref<T> { return *data(); }
        template<typename T = F> auto operator*() const noexcept -> cref<T> { return *data(); }

        operator u32() const noexcept { return idx; }
        operator bool() const noexcept { return idx != math::maxv<u32>; }

    private:
        u32 idx;
    };

}
