#pragma once
#include <metatron/core/infra/vector.hpp>
#include <algorithm>
#include <span>

namespace mtt::core {

enum struct heap : u8 {
    device,
    host,
    coherent,
    readback,
    twin,
};

template<typename T, heap h, usize alignment, bool unique>
struct buffer final {
    T* ptr = nullptr;

    buffer() noexcept = default;
    buffer(buffer const&) noexcept requires(!unique) = default;
    auto operator=(buffer const&) noexcept -> buffer& requires(!unique) = default;
    ~buffer() noexcept requires(!unique) = default;
    ~buffer() noexcept requires(unique);

    template<bool rhs_unique>
    buffer(buffer<T, h, alignment, rhs_unique> const& rhs) noexcept requires(!unique && rhs_unique);
    buffer(T* p) noexcept requires(!unique);

    buffer(usize size) noexcept requires(unique);
    template<typename U, usize extent>
    buffer(std::span<U, extent> data) noexcept requires(unique && h != heap::device && h != heap::readback);

    buffer(buffer const&) requires(unique) = delete;
    buffer(buffer&& rhs) noexcept requires(unique);
    auto operator=(buffer const&) -> buffer& requires(unique) = delete;
    auto operator=(buffer&& rhs) noexcept -> buffer& requires(unique);

    auto operator[](usize idx) const noexcept -> T const&;
    auto operator[](usize idx) noexcept -> T&;
    auto operator+(i64 offset) const noexcept -> buffer<T, h, alignment, false>;
    auto operator-(i64 offset) const noexcept -> buffer<T, h, alignment, false>;

    explicit operator u64() const noexcept;
    operator bool() const noexcept;

    auto map() noexcept -> void requires(h != heap::device && h != heap::host);
    auto unmap() noexcept -> void requires(h != heap::device && h != heap::host);
};

template<typename T, heap h, usize alignment, bool unique>
template<typename U, usize extent>
buffer<T, h, alignment, unique>::buffer(std::span<U, extent> data) noexcept requires(unique && h != heap::device && h != heap::readback)
: buffer(data.size()) {
    if constexpr (h != heap::host) map();
    std::ranges::copy(data, ptr);
    if constexpr (h != heap::host) unmap();
}

template<typename T, heap h, usize alignment, bool unique>
template<bool rhs_unique>
buffer<T, h, alignment, unique>::buffer(buffer<T, h, alignment, rhs_unique> const& rhs) noexcept requires(!unique && rhs_unique)
: ptr(rhs.ptr) {}

template<typename T, heap h, usize alignment, bool unique>
buffer<T, h, alignment, unique>::buffer(T* p) noexcept requires(!unique)
: ptr(p) {}

template<typename T, heap h, usize alignment, bool unique>
auto buffer<T, h, alignment, unique>::operator+(i64 offset) const noexcept -> buffer<T, h, alignment, false> {
    return buffer<T, h, alignment, false>{(T*)((i64)ptr + offset * (i64)sizeof(T))};
}

template<typename T, heap h, usize alignment, bool unique>
auto buffer<T, h, alignment, unique>::operator-(i64 offset) const noexcept -> buffer<T, h, alignment, false> {
    return *this + (-offset);
}

template<typename T, heap h, usize alignment, bool unique>
buffer<T, h, alignment, unique>::operator u64() const noexcept { return (u64)ptr; }
template<typename T, heap h, usize alignment, bool unique>
buffer<T, h, alignment, unique>::operator bool() const noexcept { return ptr != nullptr; }

template<typename T, heap h = heap::coherent, usize alignment = sizeof(u32)>
struct span final {
    core::buffer<T, h, alignment, false> buffer;
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

namespace mtt {

template<typename T, core::heap h = core::heap::coherent, usize alignment = sizeof(u32)>
using buf = core::buffer<T, h, alignment, true>;

}
