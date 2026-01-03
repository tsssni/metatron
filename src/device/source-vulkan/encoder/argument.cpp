#include "argument.hpp"
#include "../shader/argument.hpp"
#include "../command/buffer.hpp"
#include "../opaque/accel.hpp"
#include "../opaque/sampler.hpp"
#include "../opaque/buffer.hpp"
#include "../opaque/image.hpp"
#include "../opaque/grid.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {
    Argument_Encoder::Argument_Encoder(
        mut<command::Buffer> cmd, mut<shader::Argument> args
    ) noexcept: cmd(cmd), args(args) {}

    auto Argument_Encoder::submit() noexcept -> void {}

    auto Argument_Encoder::upload() noexcept -> void {
        encoder::Transfer_Encoder{cmd}.upload(*args->set);
    }

    auto Argument_Encoder::Impl::update(
        cref<shader::Descriptor> desc, opaque::Barrier barrier
    ) noexcept -> opaque::Barrier {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        barrier.stage = vk::PipelineStageFlagBits2::eComputeShader;
        switch (desc.type) {
        case Type::parameter:
            barrier.access = vk::AccessFlagBits2::eUniformRead;
            break;
        case Type::image:
        case Type::grid:
            switch (desc.access) {
            case Access::readonly:
                barrier.access = vk::AccessFlagBits2::eShaderSampledRead;
                barrier.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
                break;
            case Access::readwrite:
                barrier.access = vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite;
                barrier.layout = vk::ImageLayout::eGeneral;
                break;
            }
            break;
        case Type::accel:
            barrier.access = vk::AccessFlagBits2::eAccelerationStructureReadKHR;
            break;
        case Type::sampler: break;
        }
        return barrier;
    }

    template<typename T>
    auto Argument_Encoder::Impl::bind(
        mut<Argument_Encoder> encoder, std::string_view field, T view
    ) noexcept -> void {
        auto args = encoder->args;
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto readonly = desc.access == shader::Descriptor::Access::readonly;
        auto size = readonly
        ? ctx->descriptor_buffer_props.sampledImageDescriptorSize
        : ctx->descriptor_buffer_props.storageImageDescriptorSize;

        auto image = vk::DescriptorImageInfo{
            .imageView = view.ptr->impl->view.get(),
            .imageLayout = view.ptr->impl->barrier.layout,
        };
        auto info = vk::DescriptorGetInfoEXT{
            .type = readonly
            ? vk::DescriptorType::eSampledImage
            : vk::DescriptorType::eStorageImage,
            .data = readonly
            ? vk::DescriptorDataEXT{.pSampledImage = &image}
            : vk::DescriptorDataEXT{.pStorageImage = &image}
        };
        auto offset = args->impl->offsets[binding];
        args->set->dirty.push_back({offset, size});
        device.getDescriptorEXT(&info, size, args->set->ptr + offset);
    }

    template<typename T>
    auto Argument_Encoder::Impl::bind(
        mut<Argument_Encoder> encoder, std::string_view field, shader::Bindless<T> bindless
    ) noexcept -> void {
        auto args = encoder->args;
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto readonly = desc.access == shader::Descriptor::Access::readonly;
        auto size = readonly
        ? ctx->descriptor_buffer_props.sampledImageDescriptorSize
        : ctx->descriptor_buffer_props.storageImageDescriptorSize;
        auto offset = args->impl->offsets[binding] + bindless.offset * size;
        auto range = bindless.list.size() * size;
        args->set->dirty.push_back({offset, range});

        for (auto i = 0; i < bindless.list.size(); ++i) {
            auto view = bindless.list[i];
            auto image = vk::DescriptorImageInfo{
                .imageView = view.ptr->impl->view.get(),
                .imageLayout = view.ptr->impl->barrier.layout,
            };
            auto info = vk::DescriptorGetInfoEXT{
                .type = readonly
                ? vk::DescriptorType::eSampledImage
                : vk::DescriptorType::eStorageImage,
                .data = readonly
                ? vk::DescriptorDataEXT{.pSampledImage = &image}
                : vk::DescriptorDataEXT{.pStorageImage = &image}
            };
            auto ptr = args->set->ptr + offset + i * size;
            device.getDescriptorEXT(&info, size, ptr);
        }
    }

    auto Argument_Encoder::bind(std::string_view field, view<opaque::Acceleration> accel) noexcept -> void {
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto size = ctx->descriptor_buffer_props.accelerationStructureDescriptorSize;
        auto info = vk::DescriptorGetInfoEXT{
            .type = vk::DescriptorType::eAccelerationStructureKHR,
            .data = vk::DescriptorDataEXT{.accelerationStructure = accel->impl->instances_addr},
        };
        auto offset = args->impl->offsets[binding];
        args->set->dirty.push_back({offset, size});
        device.getDescriptorEXT(&info, size, args->set->ptr + offset);
    }

    auto Argument_Encoder::bind(std::string_view field, view<opaque::Sampler> sampler) noexcept -> void {
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto size = ctx->descriptor_buffer_props.samplerDescriptorSize;
        auto info = vk::DescriptorGetInfoEXT{
            .type = vk::DescriptorType::eSampler,
            .data = vk::DescriptorDataEXT{.pSampler = &sampler->impl->sampler.get()}
        };
        auto offset = args->impl->offsets[binding];
        args->set->dirty.push_back({offset, size});
        device.getDescriptorEXT(&info, size, args->set->ptr + offset);
    }

    auto Argument_Encoder::bind(std::string_view field, opaque::Image::View image) noexcept -> void { impl->bind(this, field, image); }
    auto Argument_Encoder::bind(std::string_view field, opaque::Grid::View grid) noexcept -> void { impl->bind(this, field, grid); }
    auto Argument_Encoder::bind(std::string_view field, shader::Bindless<opaque::Image> images) noexcept -> void { impl->bind(this, field, images); }
    auto Argument_Encoder::bind(std::string_view field, shader::Bindless<opaque::Grid> grids) noexcept -> void { impl->bind(this, field, grids); }

    template<typename T>
    auto Argument_Encoder::Impl::acquire(
        mut<Argument_Encoder> encoder, std::string_view field, T view
    ) noexcept -> void {
        auto args = encoder->args;
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto cmd = encoder->cmd->impl->cmd.get();

        auto state = update(desc, view.ptr->impl->barrier);
        auto barrier = view.ptr->impl->update(state);
        cmd.pipelineBarrier2({
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        });
    }

    auto Argument_Encoder::acquire(std::string_view field, std::span<byte const> uniform) noexcept -> void {
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto cmd = this->cmd->impl->cmd.get();
        auto device = ctx->device.get();
        auto size = ctx->descriptor_buffer_props.uniformBufferDescriptorSize;

        args->parameters->dirty.push_back({0, u32(uniform.size())});
        std::memcpy(args->parameters->ptr, uniform.data(), uniform.size());
        encoder::Transfer_Encoder{this->cmd}.upload(*args->parameters);

        auto state = impl->update(desc, args->parameters->impl->barrier);
        auto barrier = args->parameters->impl->update(state);
        cmd.pipelineBarrier2({
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &barrier,
        });
    }

    auto Argument_Encoder::acquire(std::string_view field, opaque::Image::View image) noexcept -> void { impl->acquire(this, field, image); }
    auto Argument_Encoder::acquire(std::string_view field, opaque::Grid::View image) noexcept -> void { impl->acquire(this, field, image); }
}
