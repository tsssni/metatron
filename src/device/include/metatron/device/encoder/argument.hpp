#pragma once
#include <metatron/device/command/buffer.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>
#include <metatron/device/shader/argument.hpp>

namespace mtt::encoder {
    struct Argument_Encoder final: stl::capsule<Argument_Encoder> {
        mut<command::Buffer> cmd;
        mut<shader::Argument> args;
        struct Impl;
        Argument_Encoder(mut<command::Buffer> cmd, mut<shader::Argument> args) noexcept;

        auto upload() noexcept -> void;
        auto bind() noexcept -> void;

        auto bind(std::string_view field, std::span<byte const> uniform) noexcept -> void;
        auto bind(std::string_view field, opaque::Image::View image) noexcept -> void;
        auto bind(std::string_view field, opaque::Grid::View grid) noexcept -> void;
        auto bind(std::string_view field, shader::Bindless<opaque::Image> images) noexcept -> void;
        auto bind(std::string_view field, shader::Bindless<opaque::Grid> grids) noexcept -> void;
        auto bind(std::string_view field, view<opaque::Sampler> sampler) noexcept -> void;

        template<typename T>
        requires std::is_aggregate_v<std::decay_t<T>> || std::is_scalar_v<std::decay_t<T>>
        auto acquire(std::string_view field, T&& uniform) noexcept -> void {
            acquire(field, {view<byte>(&uniform), sizeof(uniform)});
        }
        auto acquire(std::string_view field, std::span<byte const> uniform) noexcept -> void;
        auto acquire(std::string_view field, opaque::Image::View image) noexcept -> void;
        auto acquire(std::string_view field, opaque::Grid::View grid) noexcept -> void;
        auto acquire(std::string_view field, shader::Bindless<opaque::Image> images) noexcept -> void;
        auto acquire(std::string_view field, shader::Bindless<opaque::Grid> grids) noexcept -> void;
    };
}
