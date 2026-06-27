#include "argument.hpp"
#include <metatron/core/math/bit.hpp>

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        using Binding = vk::DescriptorType;
        auto path = (stl::path{"shader"} / desc.name).concat(".json");
        stl::json::load(stl::filesystem::find(path), reflection);

        auto total = 0u;
        auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
        for (auto i = 0u; i < reflection.size(); ++i) {
            auto constexpr types = std::to_array<Binding>({
                Binding::eInlineUniformBlock,
                Binding::eSampler,
                Binding::eSampledImage,
                Binding::eSampledImage,
                Binding::eAccelerationStructureKHR,
            });

            auto& refl = reflection[i];
            auto type = types[i32(refl.type)];
            if (type == Binding::eSampledImage && refl.access != Access::readonly)
                type = Binding::eStorageImage;

            auto count = refl.type == Type::parameter
            ? refl.size : (refl.count == 0 ? 8192 : refl.count);
            total += count;
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                .binding = i,
                .descriptorType = type,
                .descriptorCount = count,
                .stageFlags = vk::ShaderStageFlagBits::eCompute,
            });
        }

        auto& ctx = command::Context::internal();
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
            .type = command::Type::render,
            .size = size,
            .flags = 0
            | u64(vk::BufferUsageFlagBits2::eSamplerDescriptorBufferEXT)
            | u64(vk::BufferUsageFlagBits2::eResourceDescriptorBufferEXT),
        });

        impl->offsets.resize(reflection.size());
        for (auto i = 0; i < reflection.size(); ++i)
            device.getDescriptorSetLayoutBindingOffsetEXT(
                impl->layout.get(), i, &impl->offsets[i]
            );
    }
}
