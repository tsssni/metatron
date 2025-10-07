#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/compo/spectrum.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/vector.hpp>
#include <variant>

namespace mtt::compo {
    struct Constant_Spectrum_Texture final {
        ecs::Entity spectrum;
        i32 constant_spectrum{0};
    };

    struct Image_Spectrum_Texture final {
        std::string path;
        color::Color_Space::Spectrum_Type type;
        texture::Image_Distribution distr{texture::Image_Distribution::none};
        i32 image_spectrum{0};
    };

    struct Checkerboard_Texture final {
        ecs::Entity x;
        ecs::Entity y;
        math::Vector<usize, 2> uv_scale{1uz};
        i32 checkerboard{0};
    };

    using Spectrum_Texture = std::variant<
        Constant_Spectrum_Texture,
        Image_Spectrum_Texture,
        Checkerboard_Texture
    >;

    struct Constant_Vector_Texture final {
        math::Vector<f32, 4> x;
        i32 constant_vector{0};
    };

    struct Image_Vector_Texture final {
        std::string path;
        i32 image_vector{0};
    };

    using Vector_Texture = std::variant<
        Constant_Vector_Texture,
        Image_Vector_Texture
    >;

    using Texture = std::variant<
        Constant_Spectrum_Texture,
        Image_Spectrum_Texture,
        Checkerboard_Texture,
        Constant_Vector_Texture,
        Image_Vector_Texture
    >;

    using Sampler = texture::Sampler;
}

template<>
struct glz::meta<mtt::texture::Sampler::Filter> {
    using enum mtt::texture::Sampler::Filter;
    auto constexpr static value = glz::enumerate(
        none,
        nearest,
        linear
    );
};

template<>
struct glz::meta<mtt::texture::Sampler::Wrap> {
    using enum mtt::texture::Sampler::Wrap;
    auto constexpr static value = glz::enumerate(
        repeat,
        mirror,
        edge,
        border
    );
};

template<>
struct glz::meta<mtt::texture::Image_Distribution> {
    using enum mtt::texture::Image_Distribution;
    auto constexpr static value = glz::enumerate(
        none,
        uniform,
        spherical
    );
};
