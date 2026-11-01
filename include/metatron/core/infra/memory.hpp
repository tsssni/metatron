#pragma once
#include <metatron/core/infra/buffer.hpp>
#include <metatron/core/infra/command.hpp>
#include <metatron/core/infra/image.hpp>

namespace mtt::core {

struct memonc final {
    memonc(recorder& r) noexcept;
    ~memonc() noexcept;

    auto copy(contiguous auto src, contiguous auto dst) noexcept -> void;
    auto copy(contiguous auto src, tiled auto const& dst, u32 mip = 0) noexcept -> void;
    auto copy(tiled auto const& src, contiguous auto dst, u32 mip = 0) noexcept -> void;
    auto copy(tiled auto const& src, tiled auto const& dst, u32 mip = 0) noexcept -> void;

    auto generate_mipmap(tiled auto const& img) noexcept -> void;
    auto synchronize(dual auto target) noexcept -> void;
};

}
