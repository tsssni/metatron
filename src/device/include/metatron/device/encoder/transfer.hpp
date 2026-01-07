#pragma once
#include <metatron/device/command/buffer.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>

namespace mtt::encoder {
    struct Transfer_Encoder final: stl::capsule<Transfer_Encoder> {
        struct Impl;
        mut<command::Buffer> cmd;
        Transfer_Encoder(mut<command::Buffer> cmd) noexcept;

        auto submit() noexcept -> void;
        auto upload(opaque::Buffer::View buffer) noexcept -> void;
        auto upload(opaque::Image::View image) noexcept -> void;
        auto upload(opaque::Grid::View grid) noexcept -> void;

        auto persist(opaque::Buffer::View buffer) noexcept -> void;
        auto persist(opaque::Image::View buffer) noexcept -> void;
        auto persist(opaque::Grid::View buffer) noexcept -> void;

        auto transfer(opaque::Buffer::View buffer, mut<command::Queue> dst, mut<command::Queue> src) noexcept -> void;
        auto transfer(opaque::Image::View image, mut<command::Queue> dst, mut<command::Queue> src) noexcept -> void;
        auto transfer(opaque::Grid::View grid, mut<command::Queue> dst, mut<command::Queue> src) noexcept -> void;

        auto copy(opaque::Buffer::View dst, opaque::Buffer::View src) noexcept -> void;
        auto copy(opaque::Image::View dst, opaque::Buffer::View src) noexcept -> void;
        auto copy(opaque::Grid::View dst, opaque::Buffer::View src) noexcept -> void;
        auto copy(opaque::Buffer::View dst, opaque::Image::View src) noexcept -> void;
        auto copy(opaque::Buffer::View dst, opaque::Grid::View src) noexcept -> void;
        auto copy(opaque::Image::View dst, opaque::Image::View src) noexcept -> void;
        auto copy(opaque::Grid::View dst, opaque::Grid::View src) noexcept -> void;
    };
}
