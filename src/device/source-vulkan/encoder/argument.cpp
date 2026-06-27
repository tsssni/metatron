#include "argument.hpp"
#include "../shader/argument.hpp"
#include "../command/buffer.hpp"
#include "../opaque/accel.hpp"
#include "../opaque/sampler.hpp"
#include "../opaque/image.hpp"
#include "../opaque/grid.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {

    template<typename T>
    auto Argument_Encoder::Impl::identify(
        auto transform, T src, mut<shader::Argument> args, u32 base, u32 size
    ) noexcept -> void {
        auto device = command::Context::internal()->device.get();
        args->set->dirty.push_back({base, (u32)(src.size() * size)});
        for (auto k = 0u; k < src.size(); ++k) {
            auto image = vk::DescriptorImageInfo{};
            auto info = transform(src[k], image);
            device.getDescriptorEXT(&info, size, args->set->ptr + base + k * size);
        }
    }

    Argument_Encoder::Argument_Encoder(
        mut<command::Buffer> cmd, mut<shader::Argument> args
    ) noexcept: cmd(cmd), args(args) {}

    auto Argument_Encoder::submit() noexcept -> void {
        auto transfer = encoder::Transfer_Encoder{cmd};
        transfer.upload(*args->set);
        transfer.submit();
    }

    auto Argument_Encoder::push(std::span<byte const> set, uv2 range) noexcept -> void {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        auto& props = command::Context::internal()->descriptor_buffer_props;

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
            auto access = desc.access;

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
            case Type::sampler:
                impl->identify(+[](mut<opaque::Sampler> ptr, ref<vk::DescriptorImageInfo>) {
                    return vk::DescriptorGetInfoEXT{
                        .type = vk::DescriptorType::eSampler,
                        .data = vk::DescriptorDataEXT{.pSampler = &ptr->impl->sampler.get()},
                    };
                }, std::span{mut<mut<opaque::Sampler>>(src), desc.count}, args, base,
                (u32)props.samplerDescriptorSize); break;
            case Type::accel:
                impl->identify(+[](mut<opaque::Acceleration> ptr, ref<vk::DescriptorImageInfo>) {
                    return vk::DescriptorGetInfoEXT{
                        .type = vk::DescriptorType::eAccelerationStructureKHR,
                        .data = vk::DescriptorDataEXT{.accelerationStructure = ptr->impl->instances_addr},
                    };
                }, std::span{mut<mut<opaque::Acceleration>>(src), desc.count}, args, base,
                (u32)props.accelerationStructureDescriptorSize); break;
            case Type::image:
            case Type::grid: {
                auto transform = [access](auto view, ref<vk::DescriptorImageInfo> image) {
                    image = vk::DescriptorImageInfo{
                        .imageView = view.ptr->impl->view.get(),
                        .imageLayout = view.ptr->impl->barrier.layout,
                    };
                    return vk::DescriptorGetInfoEXT{
                        .type = access == Access::readonly ? vk::DescriptorType::eSampledImage : vk::DescriptorType::eStorageImage,
                        .data = vk::DescriptorDataEXT{.pSampledImage = &image},
                    };
                };
                auto size = (u32)(access == Access::readonly ? props.sampledImageDescriptorSize : props.storageImageDescriptorSize);
                if (desc.type == Type::image) impl->identify(transform,
                    std::span{mut<opaque::Image::View>(src), desc.count}, args, base, size);
                else impl->identify(transform,
                    std::span{mut<opaque::Grid::View>(src), desc.count}, args, base, size);
                break;
            }
            }
        }
    }

    auto Argument_Encoder::push(std::span<byte const> set, std::tuple<u32, std::span<byte const>> bindless) noexcept -> void {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        auto& props = command::Context::internal()->descriptor_buffer_props;

        auto [offset, span] = bindless;
        auto last = args->reflection.size() - 1;
        auto& desc = args->reflection[last];
        auto access = desc.access;
        auto size = (u32)(access == Access::readonly ? props.sampledImageDescriptorSize : props.storageImageDescriptorSize);
        auto base = (u32)args->impl->offsets[last] + offset * size;

        auto transform = [access](auto view, ref<vk::DescriptorImageInfo> image) {
            image = vk::DescriptorImageInfo{
                .imageView = view.ptr->impl->view.get(),
                .imageLayout = view.ptr->impl->barrier.layout,
            };
            return vk::DescriptorGetInfoEXT{
                .type = access == Access::readonly ? vk::DescriptorType::eSampledImage : vk::DescriptorType::eStorageImage,
                .data = vk::DescriptorDataEXT{.pSampledImage = &image},
            };
        };
        if (desc.type == Type::image) impl->identify(transform,
            std::span{mut<opaque::Image::View>(span.data()), span.size() / sizeof(opaque::Image::View)}, args, base, size);
        else impl->identify(transform,
            std::span{mut<opaque::Grid::View>(span.data()), span.size() / sizeof(opaque::Grid::View)}, args, base, size);
    }
}
