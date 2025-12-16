#include "argument.hpp"
#include "../command/buffer.hpp"
#include "../opaque/buffer.hpp"
#include "../opaque/image.hpp"
#include "../opaque/grid.hpp"
#include "../opaque/sampler.hpp"
#include <metatron/device/shader/layout.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/core/math/bit.hpp>

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept: cmd(desc.cmd) {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        using Binding = vk::DescriptorType;
        auto path = (stl::path{"shader"} / desc.name).concat(".json");
        stl::json::load(stl::filesystem::find(path), reflection);

        auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
        for (auto i = 0u; i < reflection.size(); ++i) {
            auto constexpr types = std::to_array<Binding>({
                Binding::eUniformBuffer,
                Binding::eSampler,
                Binding::eSampledImage,
                Binding::eSampledImage,
                Binding::eAccelerationStructureKHR,
            });

            auto& refl = reflection[i];
            auto type = types[i32(refl.type)];
            // compiler ensures only one uniform buffer
            if (refl.type == Type::parameter)
                parameters = make_obj<opaque::Buffer>(opaque::Buffer::Descriptor{
                    .cmd = desc.cmd,
                    .state = opaque::Buffer::State::twin,
                    .size = refl.size,
                    .flags = u64(vk::BufferUsageFlagBits2::eUniformBuffer),
                });
            if (type == Binding::eSampledImage && refl.access != Access::readonly)
                type = Binding::eStorageImage;

            table[refl.path] = i;
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                .binding = i,
                .descriptorType = type,
                .descriptorCount = math::max(1u, refl.size),
                .stageFlags = vk::ShaderStageFlagBits::eCompute,
            });
        }

        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        impl->layout = command::guard(device.createDescriptorSetLayoutUnique({
            .flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
            .bindingCount = u32(bindings.size()),
            .pBindings = bindings.data(),
        }));

        auto& props = ctx->descriptor_buffer_props;
        auto size = usize{};
        device.getDescriptorSetLayoutSizeEXT(impl->layout.get(), &size);
        set = make_obj<opaque::Buffer>(opaque::Buffer::Descriptor{
            .cmd = desc.cmd,
            .state = opaque::Buffer::State::twin,
            .size = math::align(size, 256),
            .flags = 0
            | u64(vk::BufferUsageFlagBits2::eSamplerDescriptorBufferEXT)
            | u64(vk::BufferUsageFlagBits2::eResourceDescriptorBufferEXT),
        });

        impl->offsets.resize(reflection.size());
        for (auto i = 0; i < reflection.size(); ++i) {
            auto* offset = impl->offsets.data() + i;
            device.getDescriptorSetLayoutBindingOffsetEXT(
                impl->layout.get(), i, offset
            );
            if (reflection[i].type != Type::parameter) continue;
            auto address = vk::DescriptorAddressInfoEXT{
                .address = parameters->addr,
                .range = parameters->size,
            };
            auto info = vk::DescriptorGetInfoEXT{
                .type = Binding::eUniformBuffer,
                .data = vk::DescriptorDataEXT{.pUniformBuffer = &address},
            };
            device.getDescriptorEXT(&info, props.uniformBufferDescriptorSize, set->ptr + *offset);
        }
    }

    auto Argument::Impl::barrier(cref<shader::Descriptor> desc, opaque::Barrier barrier) noexcept -> opaque::Barrier {
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
    auto Argument::Impl::bind(mut<Argument> args, std::string_view field, T view) noexcept -> void {
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
        args->set->dirty.push_back({offsets[binding], size});
        device.getDescriptorEXT(&info, size, args->set->ptr + offsets[binding]);
    }

    template<typename T>
    auto Argument::Impl::bind(mut<Argument> args, std::string_view field, Bindless<T> bindless) noexcept -> void {
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto readonly = desc.access == shader::Descriptor::Access::readonly;
        auto size = readonly
        ? ctx->descriptor_buffer_props.sampledImageDescriptorSize
        : ctx->descriptor_buffer_props.storageImageDescriptorSize;
        auto offset = offsets[binding] + bindless.offset * size;
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

    auto Argument::bind(std::string_view field, opaque::Image::View image) noexcept -> void { impl->bind(this, field, image); }
    auto Argument::bind(std::string_view field, opaque::Grid::View grid) noexcept -> void { impl->bind(this, field, grid); }
    auto Argument::bind(std::string_view field, Bindless<opaque::Image> images) noexcept -> void { impl->bind(this, field, images); }
    auto Argument::bind(std::string_view field, Bindless<opaque::Grid> grids) noexcept -> void { impl->bind(this, field, grids); }

    auto Argument::bind(std::string_view field, view<opaque::Sampler> sampler) noexcept -> void {
        auto binding = index(field);
        auto& desc = reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto readonly = desc.access == shader::Descriptor::Access::readonly;
        auto size = ctx->descriptor_buffer_props.samplerDescriptorSize;
        auto info = vk::DescriptorGetInfoEXT{
            .type = vk::DescriptorType::eSampler,
            .data = vk::DescriptorDataEXT{.pSampler = &sampler->impl->sampler.get()}
        };
        set->dirty.push_back({impl->offsets[binding], size});
        device.getDescriptorEXT(&info, size, set->ptr + impl->offsets[binding]);
    }

    template<typename T>
    auto Argument::Impl::acquire(mut<Argument> args, std::string_view field, T view) noexcept -> void {
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto cmd = args->cmd->impl->cmd.get();

        auto state = barrier(desc, view.ptr->impl->barrier);
        auto barrier = view.ptr->impl->update(state);
        cmd.pipelineBarrier2({
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        });
    }

    template<typename T>
    auto Argument::Impl::acquire(mut<Argument> args, std::string_view field, Bindless<T> bindless) noexcept -> void {
        auto binding = args->index(field);
        auto& desc = args->reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto cmd = args->cmd->impl->cmd.get();

        auto barriers = std::vector<vk::ImageMemoryBarrier2>{};
        barriers.reserve(bindless.list.size());
        for (auto view: bindless.list) {
            auto state = barrier(desc, view.ptr->impl->barrier);
            barriers.push_back(view.ptr->impl->update(state));
        }
        cmd.pipelineBarrier2({
            .imageMemoryBarrierCount = u32(barriers.size()),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    auto Argument::acquire(std::string_view field, std::span<byte const> uniform) noexcept -> void {
        auto binding = index(field);
        auto& desc = reflection[binding];
        auto& ctx = command::Context::instance().impl;
        auto cmd = this->cmd->impl->cmd.get();
        auto device = ctx->device.get();
        auto size = ctx->descriptor_buffer_props.uniformBufferDescriptorSize;

        parameters->dirty.push_back({0, u32(uniform.size())});
        std::memcpy(parameters->ptr, uniform.data(), uniform.size());
        encoder::Transfer_Encoder{this->cmd}.upload(*parameters);

        auto state = impl->barrier(desc, parameters->impl->barrier);
        auto barrier = parameters->impl->update(state);
        cmd.pipelineBarrier2({
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &barrier,
        });
    }

    auto Argument::acquire(std::string_view field, opaque::Image::View image) noexcept -> void { impl->acquire(this, field, image); }
    auto Argument::acquire(std::string_view field, opaque::Grid::View image) noexcept -> void { impl->acquire(this, field, image); }
    auto Argument::acquire(std::string_view field, Bindless<opaque::Image> images) noexcept -> void { impl->acquire(this, field, images); }
    auto Argument::acquire(std::string_view field, Bindless<opaque::Grid> grids) noexcept -> void { impl->acquire(this, field, grids); }

    auto Argument::index(std::string_view field) noexcept -> u32 {
        auto idx = table.find(field);
        if (idx == table.end()) stl::abort("field {} does not exits", field);
        return idx->second;
    }
}
