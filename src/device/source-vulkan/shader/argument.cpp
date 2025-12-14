#include "argument.hpp"
#include <metatron/device/shader/layout.hpp>
#include <metatron/core/math/bit.hpp>

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept {
        auto path = (stl::path{"shader"} / desc.name).concat(".json");
        stl::json::load(stl::filesystem::find(path), reflection);

        auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
        for (auto i = 0u; i < reflection.size(); ++i) {
            using Type = shader::Descriptor::Type;
            using Access = shader::Descriptor::Access;
            using Binding = vk::DescriptorType;
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
                uniform = make_obj<opaque::Buffer>(opaque::Buffer::Descriptor{
                    .cmd = desc.cmd,
                    .state = opaque::Buffer::State::twin,
                    .size = math::align(refl.size, 256),
                    .flags = u64(vk::BufferUsageFlagBits2::eUniformBuffer),
                });
            if (type == Binding::eSampledImage && refl.access != Access::readonly)
                type = Binding::eStorageImage;

            auto count = refl.size < 0 ? 65536u : math::max(1, refl.size);
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                .binding = i,
                .descriptorType = type,
                .descriptorCount = count,
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
            .size = size,
            .flags = 0
            | u64(vk::BufferUsageFlagBits2::eSamplerDescriptorBufferEXT)
            | u64(vk::BufferUsageFlagBits2::eResourceDescriptorBufferEXT),
        });
    }
}
