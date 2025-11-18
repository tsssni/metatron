#pragma once
#include <metatron/resource/spectra/color-space.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/stack.hpp>
#include <vector>

namespace mtt::opaque {
    struct Coordinate final {
        fv2 uv{};
        f32 dudx{0.f};
        f32 dudy{0.f};
        f32 dvdx{0.f};
        f32 dvdy{0.f};
    };

    struct Image final {
        struct Pixel final {
            Pixel(view<Image> image, mut<byte> start) noexcept;
            explicit operator fv4() const noexcept;
            auto operator=(cref<fv4> v) noexcept -> void;
            auto operator+=(cref<fv4> v) noexcept -> void;
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
            uzv4 size;
        };
        // only sRGB is supported by hardware so boolean value is enough.
        bool linear;
        // specify mip size by resizing the vector.
        // if just fill mip 0 then mipmap auto generated.
        std::vector<buf<byte>> pixels;

        auto operator[](usize x, usize y, usize lod = 0) noexcept -> Pixel;
        auto operator[](usize x, usize y, usize lod = 0) const noexcept -> Pixel const;
        auto operator()(cref<Coordinate> coord) const -> fv4;

        auto static from_path(std::string_view path, bool linear) noexcept -> Image;
        auto to_path(std::string_view path, tag<spectra::Color_Space> cs) const noexcept -> void;
    };
}
