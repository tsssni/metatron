#pragma once
#include <metatron/core/math/vector.hpp>
#include <vector>

namespace mtt::image {
    struct Coordinate final {
        math::Vector<f32, 2> uv{};
        f32 dudx{0.f};
        f32 dudy{0.f};
        f32 dvdx{0.f};
        f32 dvdy{0.f};
    };

    struct Image final {
        struct Pixel final {
            Pixel(view<Image> image, mut<byte> start) noexcept;
            explicit operator math::Vector<f32, 4>() const noexcept;
            auto operator=(math::Vector<f32, 4> const& v) noexcept -> void;
            auto operator+=(math::Vector<f32, 4> const& v) noexcept -> void;
            auto data() noexcept -> mut<byte>;
        private:
            view<Image> image;
            mut<byte> start;
        };
        friend Pixel;

        union {
            struct {
                usize width;
                usize height;
                usize channels;
                usize stride;
            };
            math::Vector<usize, 4> size;
        } const;
        // only sRGB is supported by hardware so boolean value is enough.
        bool linear;
        // specify mip size by resizing the vector.
        // if just fill mip 0 then mipmap auto generated.
        // if mip 0 empty, auto fill zero for mipmap.
        // if all mips have same size, treated as 3d image.
        std::vector<std::vector<byte>> pixels;

        auto operator[](usize x, usize y, usize lod = 0) noexcept -> Pixel;
        auto operator[](usize x, usize y, usize lod = 0) const noexcept -> Pixel const;
        auto operator()(Coordinate const& coord) const -> math::Vector<f32, 4>;
        auto operator()(math::Vector<f32, 3> const& uvw) const -> math::Vector<f32, 4>;
    };
}
