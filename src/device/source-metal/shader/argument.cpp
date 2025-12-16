#include "argument.hpp"

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept {}

    auto Argument::bind(std::string_view field, std::span<byte const> uniform) noexcept -> void {}
    auto Argument::bind(std::string_view field, opaque::Image::View image) noexcept -> void {}
    auto Argument::bind(std::string_view field, opaque::Grid::View grid) noexcept -> void {}
    auto Argument::bind(std::string_view field, Bindless<opaque::Image> images) noexcept -> void {}
    auto Argument::bind(std::string_view field, Bindless<opaque::Grid> grids) noexcept -> void {}
    auto Argument::bind(std::string_view field, view<opaque::Sampler> sampler) noexcept -> void {}

    auto Argument::acquire(std::string_view field, std::span<byte const> uniform) noexcept -> void {}
    auto Argument::acquire(std::string_view field, opaque::Image::View image) noexcept -> void {}
    auto Argument::acquire(std::string_view field, opaque::Grid::View grid) noexcept -> void {}
    auto Argument::acquire(std::string_view field, Bindless<opaque::Image> images) noexcept -> void {}
    auto Argument::acquire(std::string_view field, Bindless<opaque::Grid> grids) noexcept -> void {}
}
