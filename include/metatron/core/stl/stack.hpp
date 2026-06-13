#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/print.hpp>
#include <cstring>
#include <vector>
#include <atomic>
#include <functional>

namespace mtt::stl {
    struct buf {
        mut<byte> ptr = nullptr;
        uptr handle = 0;
        u32 bytelen = 0;
        u32 idx = math::maxv<u32>;

        auto reset() noexcept -> void {
            ptr = nullptr;
            bytelen = 0;
            idx = math::maxv<u32>;
        }
    };

    struct stack final: singleton<stack> {
        using deleter = void(*)(mut<buf>);
        auto static constexpr max_idx = 1 << 20;

        auto static push(mut<buf> buf, deleter f) noexcept -> void {
            buf->idx = instance().length.fetch_add(1, std::memory_order::relaxed);
            if (buf->idx >= max_idx) stl::abort("stack overflow");
            instance().deleters[buf->idx].store(f, std::memory_order::release);
            instance().bufs[buf->idx].store(buf, std::memory_order::release);
        }

        auto static swap(mut<buf> buf) noexcept -> void {
            if (buf->idx == math::maxv<u32>) return;
            instance().bufs[buf->idx].store(buf, std::memory_order::release);
        }

        auto static raw(u32 idx) noexcept -> mut<buf> {
            return instance().bufs[idx].load(std::memory_order::acquire);
        }

        auto static release(mut<buf> buf) noexcept -> void {
            if (buf->idx != math::maxv<u32> && instance().bufs[buf->idx].load(std::memory_order::acquire) == buf)
                instance().deleters[buf->idx].load(std::memory_order::acquire)(buf);
        }

        auto static size() noexcept -> usize { return instance().length.load(std::memory_order::relaxed); }

    private:
        std::array<std::atomic<mut<buf>>, max_idx> bufs;
        std::array<std::atomic<deleter>, max_idx> deleters;
        std::atomic<u32> length = 0;
    };
}

namespace mtt {
    template<typename T>
    struct buf final: stl::buf {
        buf() noexcept: stl::buf() {}
        ~buf() noexcept { release(); reset(); }
        buf(cref<buf> rhs) noexcept {
            ptr = rhs.ptr;
            bytelen = rhs.bytelen;
            idx = math::maxv<u32>;
        }
        buf(rref<buf> rhs) noexcept {
            std::construct_at(this, rhs);
            idx = rhs.idx;
            stl::stack::swap(this);
            rhs.reset();
        }

        operator std::span<T>() noexcept { return {data(), size()}; }
        operator std::span<T const>() const noexcept { return {data(), size()}; }
        auto operator=(cref<buf> rhs) noexcept -> ref<buf> {
            release();
            std::construct_at(this, rhs);
            return *this;
        }
        auto operator=(rref<buf> rhs) noexcept -> ref<buf> {
            release();
            std::construct_at(this, std::move(rhs));
            return *this;
        }

        buf(usize size) noexcept {
            bytelen = size * sizeof(T);
            ptr = mut<byte>(std::malloc(bytelen));
            if (!ptr) stl::abort("allocate {} bytes failed", bytelen);
            if constexpr (!std::is_trivially_constructible_v<T>)
                std::uninitialized_default_construct_n(data(), size);
            stl::stack::push(this, [](auto* ptr) {
                mut<buf>(ptr)->release();
            });
        }

        template<typename U>
        requires std::same_as<T, std::remove_const_t<U>>
        buf(std::span<U> range) noexcept: buf(range.size()) {
            std::memcpy(data(), range.data(), bytelen);
        }

        auto data() noexcept -> mut<T> { return mut<T>(ptr); }
        auto data() const noexcept -> view<T> { return view<T>(ptr); }
        auto size() const noexcept -> usize { return bytelen / sizeof(T); }
        auto empty() const noexcept -> bool { return bytelen == 0; }
        auto operator[](usize i) noexcept -> ref<T> { return data()[i]; }
        auto operator[](usize i) const noexcept -> cref<T> { return data()[i]; }

        auto subbuf(usize i, usize size) const noexcept -> buf<T> {
            auto b = buf<T>{};
            b.ptr = ptr + i * sizeof(T);
            b.bytelen = size * sizeof(T);
            return b;
        }

        auto release() noexcept -> void {
            if (idx == math::maxv<u32> || !ptr) return;
            if constexpr (!std::is_trivially_constructible_v<T>)
                std::destroy_n(data(), bytelen / sizeof(T));
            std::free(ptr);
        }
    };
}
