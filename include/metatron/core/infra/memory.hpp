#pragma once
#include <metatron/core/infra/buffer.hpp>
#include <metatron/core/infra/image.hpp>
#include <metatron/core/infra/command.hpp>

namespace mtt::infra {

template<typename T>
concept destroyable = requires(T t) { t.destroy(); };

template<destroyable T>
struct obj final {
    T handle;

    obj() noexcept = default;
    obj(T const& rhs) noexcept;
    obj(obj const&) = delete;
    obj(obj&& rhs) noexcept;
    auto operator=(obj const&) -> obj& = delete;
    auto operator=(obj&& rhs) noexcept -> obj&;
    ~obj() noexcept;
    auto operator->() noexcept -> T*;
    auto operator->() const noexcept -> T const*;
};

template<destroyable T>
obj<T>::obj(T const& rhs) noexcept: handle(rhs) {}

template<destroyable T>
obj<T>::obj(obj&& rhs) noexcept: handle(rhs.handle) { rhs.handle = T{}; }

template<destroyable T>
auto obj<T>::operator=(obj&& rhs) noexcept -> obj& {
    if (this == &rhs) return *this;
    if (handle) handle.destroy();
    handle = rhs.handle;
    rhs.handle = T{};
    return *this;
}

template<destroyable T>
obj<T>::~obj() noexcept { handle.destroy(); }

template<destroyable T>
auto obj<T>::operator->() noexcept -> T* { return &handle; }

template<destroyable T>
auto obj<T>::operator->() const noexcept -> T const* { return &handle; }

struct memonc final {
    memonc(recorder& r) noexcept;
    ~memonc() noexcept;

    auto copy(contiguous auto src, contiguous auto target) noexcept -> void;
    auto copy(contiguous auto src, tiled auto target, u32 mip = 0) noexcept -> void;
    auto copy(tiled auto src, contiguous auto target, u32 mip = 0) noexcept -> void;
    auto synchronize(dual auto target) noexcept -> void;
};

}
