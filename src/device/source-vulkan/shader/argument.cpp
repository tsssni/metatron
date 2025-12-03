#include "argument.hpp"
#include <metatron/device/shader/layout.hpp>

namespace mtt::shader {
    Argument::Impl::Impl(std::string_view name) noexcept {
        auto set = shader::Set{};
        auto path = (stl::path{"shader"} / name).concat(".json");
        stl::json::load(stl::filesystem::find(path), set);

        auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
        for (auto i = 0u; i < set.size(); ++i) {
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

            auto& desc = set[i];
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

        auto device = command::Context::instance().device;
        this->layout = command::guard(device.createDescriptorSetLayoutUnique({
            .bindingCount = u32(bindings.size()),
            .pBindings = bindings.data(),
        }));
    }

    Argument::Argument(std::string_view path) noexcept:
    stl::capsule<Argument>(path) {}
}
