#pragma once
#include <metatron/core/infra/buffer.hpp>
#include <metatron/core/infra/image.hpp>
#include <metatron/core/infra/command.hpp>

namespace mtt:: inline infra {

template<typename T>
requires requires(T t) { t.destroy(); }
struct obj final {
    T handle;

    obj() noexcept = default;
    obj(T const& rhs) noexcept: handle(rhs) {}
    obj(obj const&) = delete;
    obj(obj&& rhs) noexcept: handle(rhs.handle) { rhs.handle = T{}; }
    auto operator=(obj const&) -> obj& = delete;
    auto operator=(obj&& rhs) noexcept -> obj& {
        if (this == &rhs) return *this;
        if (handle) handle.destroy();
        handle = rhs.handle;
        rhs.handle = T{};
        return *this;
    }
    ~obj() noexcept { handle.destroy(); }
    auto operator->() noexcept -> T* { return &handle; }
    auto operator->() const noexcept -> T const* { return &handle; }
};

struct memoenc final {
    memoenc(recorder& r) noexcept;
    ~memoenc() noexcept;

    auto copy(contiguous auto src, contiguous auto target) noexcept -> void;
    auto copy(contiguous auto src, tiled auto target, u32 mip = 0) noexcept -> void;
    auto copy(tiled auto src, contiguous auto target, u32 mip = 0) noexcept -> void;
    auto synchronize(dual auto target) noexcept -> void;
};

}
