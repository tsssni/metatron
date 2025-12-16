#pragma once
#include <metatron/device/shader/layout.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>
#include <metatron/device/opaque/sampler.hpp>
#include <metatron/device/command/buffer.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::shader {
    struct Argument final: stl::capsule<Argument> {
        mut<command::Buffer> cmd;
        obj<opaque::Buffer> set;

        struct Descriptor final {
            mut<command::Buffer> cmd;
            std::string_view name;
        };

        template<typename T>
        struct Bindless final {
            usize offset;
            std::span<typename T::View> list;
        };

        struct Impl;
        Argument(cref<Descriptor> desc) noexcept;

        auto bind(std::string_view field, std::span<byte const> uniform) noexcept -> void;
        auto bind(std::string_view field, opaque::Image::View image) noexcept -> void;
        auto bind(std::string_view field, opaque::Grid::View grid) noexcept -> void;
        auto bind(std::string_view field, Bindless<opaque::Image> images) noexcept -> void;
        auto bind(std::string_view field, Bindless<opaque::Grid> grids) noexcept -> void;
        auto bind(std::string_view field, view<opaque::Sampler> sampler) noexcept -> void;

        template<typename T>
        auto acquire(std::string_view field, T&& uniform) noexcept -> void {
            acquire(field, {view<byte>(&uniform), sizeof(uniform)});
        }
        auto acquire(std::string_view field, std::span<byte const> uniform) noexcept -> void;
        auto acquire(std::string_view field, opaque::Image::View image) noexcept -> void;
        auto acquire(std::string_view field, opaque::Grid::View grid) noexcept -> void;
        auto acquire(std::string_view field, Bindless<opaque::Image> images) noexcept -> void;
        auto acquire(std::string_view field, Bindless<opaque::Grid> grids) noexcept -> void;


    private:
        auto index(std::string_view field) noexcept -> u32;

        Set reflection;
        obj<opaque::Buffer> parameters;
        stl::table<u32> table;

    };
}
