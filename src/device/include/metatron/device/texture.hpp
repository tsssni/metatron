#pragma once
#include <metatron/device/sampler.hpp>
#include <metatron/core/math/vector.hpp>
#include <vector>

namespace mtt::device {
    struct Coordinate final {
        math::Vector<f32, 2> uv{};
        f32 dudx{0.f};
        f32 dudy{0.f};
        f32 dvdx{0.f};
        f32 dvdy{0.f};
    };

    struct Texture final {
        struct Pixel final {
            Pixel(view<Texture> image, mut<byte> start) noexcept;
            explicit operator math::Vector<f32, 4>() const noexcept;
            auto operator=(math::Vector<f32, 4> const& v) noexcept -> void;
            auto operator+=(math::Vector<f32, 4> const& v) noexcept -> void;
        private:
            view<Texture> image;
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
        // just fill mip 0, others auto generated.
        // if mip 0 empty, auto fill zero.
        std::vector<std::vector<byte>> pixels;

        auto operator[](usize x, usize y, usize lod = 0) noexcept -> Pixel;
        auto operator[](usize x, usize y, usize lod = 0) const noexcept -> Pixel const;
        auto operator()(Coordinate const& coord) const -> math::Vector<f32, 4>;
    };
}
