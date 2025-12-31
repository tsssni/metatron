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
    Argument::Argument(cref<Descriptor> desc) noexcept {
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
                parameters = make_desc<opaque::Buffer>({
                    .state = opaque::Buffer::State::twin,
                    .type = desc.type,
                    .size = refl.size,
                    .flags = u64(vk::BufferUsageFlagBits2::eUniformBuffer),
                });
            if (type == Binding::eSampledImage && refl.access != Access::readonly)
                type = Binding::eStorageImage;

            table[refl.path] = i;
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                .binding = i,
                .descriptorType = type,
                .descriptorCount = math::max(1u,
                refl.type == Type::parameter ? 1 : refl.size),
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
        set = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::twin,
            .type = desc.type,
            .size = size,
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
            auto size = props.uniformBufferDescriptorSize;
            set->dirty.push_back({*offset, size});
            device.getDescriptorEXT(&info, size, set->ptr + *offset);
        }
    }

    auto Argument::index(std::string_view field) noexcept -> u32 {
        auto idx = table.find(field);
        if (idx == table.end()) stl::abort("field {} does not exits", field);
        return idx->second;
    }
}
