#pragma once
#include <metatron/resource/shape/sphere.hpp>
#include <metatron/resource/eval/context.hpp>

namespace mtt::texture {
    struct Coordinate final {
        math::Vector<f32, 2> uv{};
        f32 dudx{0.f};
        f32 dudy{0.f};
        f32 dvdx{0.f};
        f32 dvdy{0.f};
    };

    struct Sampler final {
        enum struct Filter: usize {
            none,
            nearest,
            linear,
        };

        enum struct Wrap: usize {
            repeat,
            mirror,
            edge,
            border,
        };

        Filter min_filter{Filter::linear};
        Filter mag_filter{Filter::linear};
        Filter mip_filter{Filter::linear};

        Wrap wrap_u{Wrap::repeat};
        Wrap wrap_v{Wrap::repeat};
        Wrap wrap_w{Wrap::repeat};

        float anisotropy{16.f};
        float lod_bias{0.f};
        float min_lod{0.f};
        float max_lod{1024.f};

        math::Vector<f32, 4> border{0.f};
    };

    MTT_POLY_METHOD(texture_sample, sample);
    MTT_POLY_METHOD(texture_pdf, pdf);

    struct Spectrum_Texture final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        Sampler const& sampler,
        Coordinate const& coord,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> spectra::Stochastic_Spectrum>
    ::add_convention<texture_sample, auto (
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> math::Vector<f32, 2>>
    ::add_convention<texture_pdf, auto (
        math::Vector<f32, 2> const& uv
    ) const noexcept -> f32>
    ::template add_skill<pro::skills::as_view>
    ::build {};

    struct Vector_Texture final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        Sampler const& sampler,
        Coordinate const& coord
    ) const noexcept -> math::Vector<f32, 4>>
    ::add_convention<texture_sample, auto (
        eval::Context const& ctx,
        math::Vector<f32, 2> u
    ) const noexcept -> math::Vector<f32, 2>>
    ::add_convention<texture_pdf, auto (
        math::Vector<f32, 2> uv
    ) const noexcept -> f32>
    ::template add_skill<pro::skills::as_view>
    ::build {};

    auto grad(
        math::Ray_Differential const& diff,
        shape::Interaction const& intr
    ) noexcept -> std::optional<texture::Coordinate>;
}
