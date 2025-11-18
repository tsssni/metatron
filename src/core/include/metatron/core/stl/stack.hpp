#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/math/constant.hpp>
#include <cstring>
#include <vector>

namespace mtt::stl {
    struct buf {
        mut<byte> ptr = nullptr;
        u32 bytelen = 0u;
        u32 idx = math::maxv<u32>;
    };

    struct stack final: singleton<stack> {
        std::vector<mut<buf>> bufs;
        std::mutex mutex;

        auto push(mut<buf> buf) noexcept {
            auto lock = std::lock_guard{mutex};
            buf->idx = bufs.size();
            bufs.push_back(buf);
            if (bufs.size() >= math::maxv<u32>) {
                std::println("stack overflow");
                std::abort();
            }
        }

        auto swap(mut<buf> buf) noexcept -> void {
            auto bid = buf->idx;
            if (bid == math::maxv<u32>) return;
            auto lock = std::lock_guard{mutex};
            bufs[bid] = buf;
        }
    };
}

namespace mtt {
    template<typename T>
    struct buf final: stl::buf {
        buf() noexcept: stl::buf() {}
        ~buf() noexcept { free(); }
        buf(cref<buf> rhs) noexcept { *this = rhs; };
        buf(rref<buf> rhs) noexcept { *this = std::move(rhs); }

        operator std::span<T>() noexcept { return {data(), size()}; }
        operator std::span<T const>() const noexcept { return {data(), size()}; }
        auto operator=(cref<buf> rhs) noexcept -> ref<buf> {
            free();
            ptr = rhs.ptr;
            bytelen = rhs.bytelen;
            idx = math::maxv<u32>;
            return *this;
        }
        auto operator=(rref<buf> rhs) noexcept -> ref<buf> {
            *this = rhs;
            rhs.reset();
            stl::stack::instance().swap(this);
            return *this;
        }

        buf(usize size) noexcept {
            bytelen = size * sizeof(T);
            ptr = mut<byte>(std::malloc(bytelen));
            if (!ptr) {
                std::println("allocate {} bytes failed", bytelen);
                std::abort();
            }
            if constexpr (!std::is_trivially_constructible_v<T>)
                std::uninitialized_default_construct_n(data(), size);
            stl::stack::instance().push(this);
        }
        template<typename U>
        requires std::is_same_v<T, std::remove_const_t<U>>
        buf(std::span<U> range) noexcept: buf(range.size()) {
            std::memcpy(data(), range.data(), bytelen);
        }

        auto data() noexcept -> mut<T> { return mut<T>(ptr); }
        auto data() const noexcept -> view<T> { return view<T>(ptr); }
        auto size() const noexcept -> usize { return bytelen / sizeof(T); }
        auto empty() const noexcept -> bool { return bytelen == 0; }
        auto operator[](usize i) noexcept -> ref<T> { return data()[i]; }
        auto operator[](usize i) const noexcept -> cref<T> { return data()[i]; }

        auto subbuf(usize i, usize size) noexcept -> buf<T> {
            auto b = buf<T>{};
            b.ptr = ptr + i * sizeof(T);
            b.bytelen = size * sizeof(T);
            return b;
        }

    private:
        auto reset() noexcept -> void { ptr = nullptr; bytelen = 0; idx = math::maxv<u32>; }
        auto free() noexcept -> void {
            if (idx != math::maxv<u32> && ptr) {
                if constexpr (!std::is_trivially_constructible_v<T>)
                    std::destroy_n(data(), bytelen / sizeof(T));
                std::free(ptr);
            }
        }
    };
}
