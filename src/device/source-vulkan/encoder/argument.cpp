#include "argument.hpp"
#include "../shader/argument.hpp"
#include "../command/buffer.hpp"
#include "../opaque/accel.hpp"
#include "../opaque/sampler.hpp"
#include "../opaque/image.hpp"
#include "../opaque/grid.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {
    Argument_Encoder::Argument_Encoder(
        mut<command::Buffer> cmd, mut<shader::Argument> args
    ) noexcept: cmd(cmd), args(args) {}

    auto Argument_Encoder::submit() noexcept -> void {
        auto transfer = encoder::Transfer_Encoder{cmd};
        transfer.upload(*args->set);
        transfer.submit();
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

    auto Argument_Encoder::push(std::span<byte const> set, uv2 range) noexcept -> void {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        auto& ctx = command::Context::internal();
        auto device = ctx->device.get();
        auto& props = ctx->descriptor_buffer_props;

        auto begin = range[0];
        auto end = range[0] + range[1];
        auto first = std::ranges::upper_bound(
            args->reflection, begin, {}, &shader::Descriptor::offset
        ) - std::ranges::begin(args->reflection) - 1;

        for (auto i = first; i < args->reflection.size(); ++i) {
            auto& desc = args->reflection[i];
            if (desc.offset >= end) break;
            if (desc.count == 0) continue;
            auto src = set.data() + desc.offset;
            auto base = (u32)args->impl->offsets[i];

            switch (desc.type) {
            case Type::parameter: {
                auto lo = math::max(begin, desc.offset);
                auto hi = math::min(end, desc.offset + desc.size);
                auto offset = base + (lo - desc.offset);
                auto size = hi - lo;
                std::memcpy(args->set->ptr + offset, set.data() + lo, size);
                args->set->dirty.push_back({offset, size});
                break;
            }
            case Type::sampler: {
                auto sampler = *mut<mut<opaque::Sampler>>(src);
                auto size = (u32)props.samplerDescriptorSize;
                auto info = vk::DescriptorGetInfoEXT{
                    .type = vk::DescriptorType::eSampler,
                    .data = vk::DescriptorDataEXT{.pSampler = &sampler->impl->sampler.get()},
                };
                args->set->dirty.push_back({base, size});
                device.getDescriptorEXT(&info, size, args->set->ptr + base);
                break;
            }
            case Type::accel: {
                auto accel = *mut<mut<opaque::Acceleration>>(src);
                auto size = (u32)props.accelerationStructureDescriptorSize;
                auto info = vk::DescriptorGetInfoEXT{
                    .type = vk::DescriptorType::eAccelerationStructureKHR,
                    .data = vk::DescriptorDataEXT{.accelerationStructure = accel->impl->instances_addr},
                };
                args->set->dirty.push_back({base, size});
                device.getDescriptorEXT(&info, size, args->set->ptr + base);
                break;
            }
            case Type::image:
            case Type::grid: {
                auto readonly = desc.access == Access::readonly;
                auto size = (u32)(readonly ? props.sampledImageDescriptorSize : props.storageImageDescriptorSize);
                auto encode = [&]<typename V>(std::type_identity<V>) {
                    args->set->dirty.push_back({base, desc.count * size});
                    for (auto k = 0u; k < desc.count; ++k) {
                        auto view = mut<V>(src) + k;
                        auto image = vk::DescriptorImageInfo{
                            .imageView = view->ptr->impl->view.get(),
                            .imageLayout = view->ptr->impl->barrier.layout,
                        };
                        auto info = vk::DescriptorGetInfoEXT{
                            .type = readonly ? vk::DescriptorType::eSampledImage : vk::DescriptorType::eStorageImage,
                            .data = readonly ? vk::DescriptorDataEXT{.pSampledImage = &image} : vk::DescriptorDataEXT{.pStorageImage = &image},
                        };
                        device.getDescriptorEXT(&info, size, args->set->ptr + base + k * size);
                    }
                };
                if (desc.type == Type::image) encode(std::type_identity<opaque::Image::View>{});
                else encode(std::type_identity<opaque::Grid::View>{});
                break;
            }
            }
        }
    }

    auto Argument_Encoder::push(
        std::span<byte const>, u32 offset, std::span<byte const> bindless
    ) noexcept -> void {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        auto& ctx = command::Context::internal();
        auto device = ctx->device.get();
        auto& props = ctx->descriptor_buffer_props;

        auto last = args->reflection.size() - 1;
        auto& desc = args->reflection[last];
        auto readonly = desc.access == Access::readonly;
        auto size = (u32)(readonly ? props.sampledImageDescriptorSize : props.storageImageDescriptorSize);
        auto base = (u32)args->impl->offsets[last] + offset * size;

        auto encode = [&]<typename V>(std::type_identity<V>) {
            auto n = (u32)(bindless.size() / sizeof(V));
            args->set->dirty.push_back({base, n * size});
            for (auto k = 0u; k < n; ++k) {
                auto view = mut<V>(bindless.data()) + k;
                auto image = vk::DescriptorImageInfo{
                    .imageView = view->ptr->impl->view.get(),
                    .imageLayout = view->ptr->impl->barrier.layout,
                };
                auto info = vk::DescriptorGetInfoEXT{
                    .type = readonly ? vk::DescriptorType::eSampledImage : vk::DescriptorType::eStorageImage,
                    .data = readonly ? vk::DescriptorDataEXT{.pSampledImage = &image} : vk::DescriptorDataEXT{.pStorageImage = &image},
                };
                device.getDescriptorEXT(&info, size, args->set->ptr + base + k * size);
            }
        };
        if (desc.type == Type::image) encode(std::type_identity<opaque::Image::View>{});
        else encode(std::type_identity<opaque::Grid::View>{});
    }
}
