#include "transfer.hpp"
#include "../command/buffer.hpp"

namespace mtt::encoder {
    Transfer_Encoder::Transfer_Encoder(mut<command::Buffer> cmd) noexcept: cmd(cmd) {
        // impl->encoder = cmd->impl->cmd->blitCommandEncoder();
    }
    auto Transfer_Encoder::submit() noexcept -> void { impl->encoder->endEncoding(); }

    auto Transfer_Encoder::upload(opaque::Buffer::View buffer) noexcept -> void {}
    auto Transfer_Encoder::upload(opaque::Image::View image) noexcept -> void {}
    auto Transfer_Encoder::upload(opaque::Grid::View grid) noexcept -> void {}

    auto Transfer_Encoder::copy(opaque::Buffer::View dst, opaque::Buffer::View src) noexcept -> void {}
    auto Transfer_Encoder::copy(opaque::Image::View dst, opaque::Buffer::View src) noexcept -> void {}
    auto Transfer_Encoder::copy(opaque::Grid::View dst, opaque::Buffer::View src) noexcept -> void {}
    auto Transfer_Encoder::copy(opaque::Buffer::View dst, opaque::Image::View src) noexcept -> void {}
    auto Transfer_Encoder::copy(opaque::Buffer::View dst, opaque::Grid::View src) noexcept -> void {}
    auto Transfer_Encoder::copy(opaque::Image::View dst, opaque::Image::View src) noexcept -> void {}
    auto Transfer_Encoder::copy(opaque::Grid::View dst, opaque::Grid::View src) noexcept -> void {}

    // Vulkan needs these

    auto Transfer_Encoder::persist(opaque::Buffer::View buffer) noexcept -> void {}
    auto Transfer_Encoder::persist(opaque::Image::View image) noexcept -> void {}
    auto Transfer_Encoder::persist(opaque::Grid::View grid) noexcept -> void {}

    auto Transfer_Encoder::acquire(opaque::Buffer::View buffer) noexcept -> void {}
    auto Transfer_Encoder::acquire(opaque::Image::View image) noexcept -> void {}
    auto Transfer_Encoder::acquire(opaque::Grid::View grid) noexcept -> void {}

    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Buffer::View buffer) noexcept -> void {}
    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Image::View image) noexcept -> void {}
    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Grid::View grid) noexcept -> void {}

}
