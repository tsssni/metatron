#include "argument.hpp"
#include <metatron/device/shader/layout.hpp>

namespace mtt::shader {
    Argument::Impl::Impl(std::string_view name) noexcept {
        auto reflection = shader::Set{};
        auto path = (stl::path{"shader"} / name).concat(".json");
        stl::json::load(stl::filesystem::find(path), reflection);

        auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
        for (auto i = 0u; i < reflection.size(); ++i) {
            using Type = Descriptor::Type;
            using Access = Descriptor::Access;
            using Vk_Type = vk::DescriptorType;
            auto constexpr types = std::to_array<vk::DescriptorType>({
                Vk_Type::eUniformBuffer,
                Vk_Type::eSampler,
                Vk_Type::eSampledImage,
                Vk_Type::eSampledImage,
                Vk_Type::eAccelerationStructureKHR,
            });

            auto& desc = reflection[i];
            auto type = types[i32(desc.type)];
            if (type == Vk_Type::eSampledImage && desc.access != Access::readonly)
                type = Vk_Type::eStorageImage;
            auto count = desc.size < 0 ? 65536u : math::max(1, desc.size);
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                .binding = i,
                .descriptorType = type,
                .descriptorCount = count,
                .stageFlags = vk::ShaderStageFlagBits::eCompute,
            });
        }

        auto device = command::Context::instance().impl->device.get();
        layout = command::guard(device.createDescriptorSetLayoutUnique({
            .flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
            .bindingCount = u32(bindings.size()),
            .pBindings = bindings.data(),
        }));

        auto size = vk::DeviceSize{};
        device.getDescriptorSetLayoutSizeEXT(layout.get(), &size);
        set = make_obj<opaque::Buffer>(stl::buf{
            .bytelen = u32(size),
            .flags = 0
            | u32(vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT)
            | u32(vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT),
            .visible = 0
        });
        for (auto i = 0; i < command::Context::Impl::ring_count; ++i) {
            buffer[i].resize(size);
            imported[i] = make_obj<opaque::Buffer>(stl::buf{
                .host = buffer[i].data(),
                .bytelen = u32(buffer.size()),
                .visible = 1,
            });
        }
    }

    Argument::Argument(std::string_view path) noexcept:
    stl::capsule<Argument>(path) {}
}
