#include "context.hpp"
#include "../shader/argument.hpp"
#include "../shader/pipeline.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::command {
    Context::Impl::Impl() noexcept {
        init_instance();
        init_device();
        init_pipeline_cache();
    }

    Context::Impl::~Impl() noexcept {
        auto path = stl::filesystem::instance().cache / "pipeline.bin";
    }

    auto Context::Impl::init_instance() noexcept -> void {
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
    }

    auto Context::Impl::init_device() noexcept -> void {
        auto chain = vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceBufferDeviceAddressFeatures,
            vk::PhysicalDeviceDescriptorIndexingFeatures,
            vk::PhysicalDeviceDescriptorBufferFeaturesEXT,
            vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
            vk::PhysicalDeviceRayQueryFeaturesKHR
        >{};
        auto& address = chain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
        address.bufferDeviceAddress = true;
        auto& bindless = chain.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();
        bindless.shaderSampledImageArrayNonUniformIndexing = true;
        bindless.shaderStorageImageArrayNonUniformIndexing = true;
        bindless.runtimeDescriptorArray = true;
        auto& buffer = chain.get<vk::PhysicalDeviceDescriptorBufferFeaturesEXT>();
        buffer.descriptorBuffer = true;
        auto& accel = chain.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();
        accel.accelerationStructure = true;
        auto& query = chain.get<vk::PhysicalDeviceRayQueryFeaturesKHR>();
        query.rayQuery = true;

        auto constexpr versions = std::to_array({
            vk::makeApiVersion(0, 1, 0, 0),
            vk::makeApiVersion(0, 1, 1, 0),
            vk::makeApiVersion(0, 1, 2, 0),
            vk::makeApiVersion(0, 1, 3, 0),
            vk::makeApiVersion(0, 1, 4, 0),
        });
        auto required_extensions = std::to_array({
            std::vector<view<char>>{},
            std::vector<view<char>>{},
            std::vector<view<char>>{
                "VK_KHR_buffer_device_address",
            },
            std::vector<view<char>>{},
            std::vector<view<char>>{
                "VK_EXT_descriptor_indexing",
                "VK_EXT_descriptor_buffer",
                "VK_KHR_deferred_host_operations",
                "VK_KHR_acceleration_structure",
                "VK_KHR_ray_query",
            },
        });

        auto render_info = vk::DeviceQueueCreateInfo{.queueCount = 0};
        auto copy_info = vk::DeviceQueueCreateInfo{.queueCount = 0};
        for (auto&& physical_device: guard(instance->enumeratePhysicalDevices())) {
            auto families = physical_device.getQueueFamilyProperties2();
            for (auto i = 0; i < families.size(); ++i) {
                auto const& family = families[i];
                auto& props = family.queueFamilyProperties;
                auto flags = props.queueFlags;
                if (render_info.queueCount == 0 && flags & vk::QueueFlags::BitsType::eGraphics)
                    render_info = {.queueFamilyIndex = u32(i), .queueCount = props.queueCount};
                else if (copy_info.queueCount == 0 && flags & vk::QueueFlags::BitsType::eTransfer)
                    copy_info = {.queueFamilyIndex = u32(i), .queueCount = props.queueCount};
                if (render_info.queueCount && copy_info.queueCount) break;
            }

            if (!render_info.queueCount || !copy_info.queueCount) {
                std::println("no vulkan async copy queue");
                std::abort();
            }
            auto render_priorities = std::vector<f32>(render_info.queueCount);
            auto copy_priorities = std::vector<f32>(copy_info.queueCount);
            for (auto i = 0; i < render_info.queueCount; ++i)
                render_priorities[i] = f32(i) / render_info.queueCount;
            for (auto i = 0; i < copy_info.queueCount; ++i)
                copy_priorities[i] = f32(i) / copy_info.queueCount;
            render_info.pQueuePriorities = render_priorities.data();
            copy_info.pQueuePriorities = copy_priorities.data();
            auto queues = std::vector{render_info, copy_info};

            auto version = vk::apiVersionMinor(physical_device.getProperties2().properties.apiVersion);
            auto extensions = std::vector<view<char>>{};
            for (auto i = 0; i < versions.size(); ++i)
                if (version <= vk::apiVersionMinor(versions[i]))
                    std::ranges::copy(required_extensions[i], std::back_inserter(extensions));

            auto info = vk::DeviceCreateInfo{};
            info.setPEnabledExtensionNames(extensions);
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
        for (auto i = 0u; i < render_info.queueCount; ++i)
            render_queues.push(device->getQueue2({
                .queueFamilyIndex = render_info.queueFamilyIndex,
                .queueIndex = i,
            }));
        for (auto i = 0u; i < copy_info.queueCount; ++i)
            copy_queues.push(device->getQueue2({
                .queueFamilyIndex = copy_info.queueFamilyIndex,
                .queueIndex = i,
            }));
    }

    auto Context::Impl::init_pipeline_cache() noexcept -> void {
        auto cache_opt = stl::filesystem::hit("pipeline.bin");
        auto cache = std::vector<byte>{};
        if (cache_opt) cache = stl::filesystem::load(*cache_opt, std::ios::binary);
        pipeline_cache = guard(device->createPipelineCacheUnique({
            .initialDataSize = cache.size(),
            .pInitialData = cache.data(),
        }));
    }

    auto Context::init() noexcept -> void {
        Context::instance();
        auto global = shader::Argument{"trace.global"};
        auto argi = shader::Argument{"trace.integrate.in"};
        auto argp = shader::Argument{"trace.postprocess.in"};
        auto ppli = shader::Pipeline{"trace.integrate", {&global, &argi}};
        auto pplp = shader::Pipeline{"trace.postprocess", {&global, &argp}};
    }
}
