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

        auto acquire(opaque::Buffer::View buffer) noexcept -> void;
        auto acquire(opaque::Image::View image) noexcept -> void;
        auto acquire(opaque::Grid::View grid) noexcept -> void;

        auto release(mut<command::Buffer> dst, opaque::Buffer::View buffer) noexcept -> void;
        auto release(mut<command::Buffer> dst, opaque::Image::View image) noexcept -> void;
        auto release(mut<command::Buffer> dst, opaque::Grid::View grid) noexcept -> void;

        auto copy(opaque::Buffer::View dst, opaque::Buffer::View src) noexcept -> void;
        auto copy(opaque::Image::View dst, opaque::Buffer::View src) noexcept -> void;
        auto copy(opaque::Grid::View dst, opaque::Buffer::View src) noexcept -> void;
        auto copy(opaque::Buffer::View dst, opaque::Image::View src) noexcept -> void;
        auto copy(opaque::Buffer::View dst, opaque::Grid::View src) noexcept -> void;
        auto copy(opaque::Image::View dst, opaque::Image::View src) noexcept -> void;
        auto copy(opaque::Grid::View dst, opaque::Grid::View src) noexcept -> void;
    };
}
