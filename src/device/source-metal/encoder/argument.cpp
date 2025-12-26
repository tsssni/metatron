#include "argument.hpp"

namespace mtt::encoder {
    Argument_Encoder::Argument_Encoder(mut<command::Buffer> cmd, mut<shader::Argument> args) noexcept {}

    auto Argument_Encoder::upload() noexcept -> void {}

    auto Argument_Encoder::bind(std::string_view field, view<opaque::Acceleration> accel) noexcept -> void {}
    auto Argument_Encoder::bind(std::string_view field, view<opaque::Sampler> sampler) noexcept -> void {}
    auto Argument_Encoder::bind(std::string_view field, opaque::Image::View image) noexcept -> void {}
    auto Argument_Encoder::bind(std::string_view field, opaque::Grid::View grid) noexcept -> void {}
    auto Argument_Encoder::bind(std::string_view field, shader::Bindless<opaque::Image> images) noexcept -> void {}
    auto Argument_Encoder::bind(std::string_view field, shader::Bindless<opaque::Grid> grids) noexcept -> void {}

    auto Argument_Encoder::acquire(std::string_view field, std::span<byte const> uniform) noexcept -> void {}
    auto Argument_Encoder::acquire(std::string_view field, opaque::Image::View image) noexcept -> void {}
    auto Argument_Encoder::acquire(std::string_view field, opaque::Grid::View grid) noexcept -> void {}
}
