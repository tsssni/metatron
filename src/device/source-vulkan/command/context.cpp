#include "context.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::command {
    auto constexpr vulkan_versions = std::to_array({
        vk::makeApiVersion(0, 1, 0, 0),
        vk::makeApiVersion(0, 1, 1, 0),
        vk::makeApiVersion(0, 1, 2, 0),
        vk::makeApiVersion(0, 1, 3, 0),
        vk::makeApiVersion(0, 1, 4, 0),
    });

    struct Context::Impl final {
        vk::UniqueInstance instance;
        vk::UniqueDevice device;
        u32 render_queue_family; u32 render_queue_size = 0;
        u32 copy_queue_family; u32 copy_queue_size = 0;

        Impl() noexcept {
            auto app = vk::ApplicationInfo{
                .pApplicationName = "metatron",
                .applicationVersion = vk::makeApiVersion(0, 0, 2, 0),
                .pEngineName = "metatron",
                .engineVersion = vk::makeApiVersion(0, 0, 2, 0),
                .apiVersion = vk::makeApiVersion(0, 1, 4, 328),
            };

            auto layers = std::vector<view<char>>{};
            for (auto&& prop: guard(vk::enumerateInstanceLayerProperties())) {
                auto constexpr validation_layer = "VK_LAYER_KHRONOS_validation";
                if (std::string_view{prop.layerName} == validation_layer)
                    layers.push_back(validation_layer);
            }
            instance = guard(vk::createInstanceUnique({
                .pApplicationInfo = &app,
                .enabledLayerCount = u32(layers.size()),
                .ppEnabledLayerNames = layers.data(),
            }));

            auto chain = vk::StructureChain<
                vk::PhysicalDeviceFeatures2,
                vk::PhysicalDeviceBufferDeviceAddressFeatures,
                vk::PhysicalDeviceDescriptorIndexingFeatures,
                vk::PhysicalDeviceRayQueryFeaturesKHR
            >{};
            auto& address = chain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
            address.bufferDeviceAddress = true;
            auto& bindless = chain.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();
            bindless.shaderSampledImageArrayNonUniformIndexing = true;
            bindless.shaderStorageImageArrayNonUniformIndexing = true;
            auto& query = chain.get<vk::PhysicalDeviceRayQueryFeaturesKHR>();
            query.rayQuery = true;

            for (auto&& physical_device: guard(instance->enumeratePhysicalDevices())) {
                auto families = physical_device.getQueueFamilyProperties2();
                for (auto i = 0; i < families.size(); ++i) {
                    auto const& family = families[i];
                    auto& props = family.queueFamilyProperties;
                    auto flags = props.queueFlags;
                    if (render_queue_size == 0 && flags & vk::QueueFlags::BitsType::eGraphics) {
                        render_queue_family = i;
                        render_queue_size = props.queueCount;
                    } else if (copy_queue_size == 0 && flags & vk::QueueFlags::BitsType::eTransfer) {
                        copy_queue_family = i;
                        copy_queue_size = props.queueCount;
                    }
                    if (render_queue_size && copy_queue_size) break;
                }

                if (!render_queue_size || !copy_queue_size) {
                    std::println("no vulkan async copy queue");
                    std::abort();
                }
                auto render_priorities = std::vector<f32>(render_queue_size);
                auto copy_priorities = std::vector<f32>(copy_queue_size);
                for (auto i = 0; i < render_queue_size; ++i)
                    render_priorities[i] = f32(i) / render_queue_size;
                for (auto i = 0; i < copy_queue_size; ++i)
                    copy_priorities[i] = f32(i) / copy_queue_size;

                auto queues = std::vector<vk::DeviceQueueCreateInfo>{
                    {
                        .queueFamilyIndex = render_queue_family,
                        .queueCount = render_queue_size,
                        .pQueuePriorities = render_priorities.data(),
                    },
                    {
                        .queueFamilyIndex = copy_queue_family,
                        .queueCount = copy_queue_size,
                        .pQueuePriorities = copy_priorities.data(),
                    },
                };
                auto info = vk::DeviceCreateInfo{};
                info.setQueueCreateInfos(queues);
                info.setPNext(chain.get());

                if (auto device = physical_device.createDeviceUnique(info);
                    device.result == vk::Result::eSuccess
                ) { this->device = std::move(device.value); break; }
            }

            if (!device) {
                std::println("no vulkan device meets requirements");
                std::abort();
            }
        }
    };

    Context::Context() noexcept {
        device = impl->device.get();
        render_queue_family = impl->render_queue_family;
        render_queue_size = impl->render_queue_size;
        copy_queue_family = impl->copy_queue_family;
        copy_queue_size = impl->copy_queue_size;
    }

    auto init() noexcept -> void {
        Context::instance();
    }
}
