#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt:: inline infra {

enum struct heap: u8 { device, host, coherent, readback, twin };

template<typename T, heap h, usize alignment, bool unique>
struct buffer final {
    T* ptr = nullptr;

    buffer() noexcept = default;
    buffer(buffer const&) noexcept requires(!unique) = default;
    auto operator=(buffer const&) noexcept -> buffer& requires(!unique) = default;
    ~buffer() noexcept requires(!unique) = default;
    buffer(buffer<T, h, alignment, true> const& rhs) noexcept requires(!unique): ptr(rhs.ptr) {}
    buffer(T* p) noexcept requires(!unique): ptr(p) {}

    buffer(usize size) noexcept requires(unique);
    buffer(buffer const&) requires(unique) = delete;
    auto operator=(buffer const&) -> buffer& requires(unique) = delete;
    buffer(buffer&& rhs) noexcept requires(unique);
    auto operator=(buffer&& rhs) noexcept -> buffer& requires(unique);
    ~buffer() noexcept requires(unique);

    auto operator[](usize idx) const noexcept -> T const&;
    auto operator[](usize idx) noexcept -> T&;
    auto operator+(i64 offset) const noexcept -> buffer<T, h, alignment, false> {
        return buffer<T, h, alignment, false>{(T*)((i64)ptr + offset * (i64)sizeof(T))};
    }
    auto operator-(i64 offset) const noexcept -> buffer<T, h, alignment, false> {
        return *this + (-offset);
    }

    explicit operator u64() const noexcept { return (u64)ptr; }
    operator bool() const noexcept { return ptr != nullptr; }

    auto map() noexcept -> void requires(h != heap::device && h != heap::host);
    auto unmap() noexcept -> void requires(h != heap::device && h != heap::host);
};

template<typename T, heap h, usize alignment = sizeof(fv4)>
using buf = buffer<T, h, alignment, true>;

template<typename T, heap h, usize alignment = sizeof(fv4)>
using ptr = buffer<T, h, alignment, false>;

template<typename T, heap h, usize alignment = sizeof(fv4)>
struct span final {
    ptr<T, h, alignment> buffer;
    usize size;
};

template<typename B>
concept pointer = requires {
    []<typename T, heap h, usize alignment>(buffer<T, h, alignment, false> const*) {}((B const*)nullptr);
};

template<typename S>
concept dual = requires {
    []<typename T, usize alignment>(span<T, heap::twin, alignment> const*) {}((S const*)nullptr);
};

template<typename S>
concept contiguous = requires {
    []<typename T, heap h, usize alignment>(span<T, h, alignment> const*) {}((S const*)nullptr);
};

}
