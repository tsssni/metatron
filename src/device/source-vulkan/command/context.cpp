#include "context.hpp"
#include "queue.hpp"
#include "../shader/argument.hpp"
#include "../shader/pipeline.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/math/vector.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mtt::command {
    Context::Impl::Impl() noexcept {
        init_instance();
        init_device();
        init_memory();
        init_pipeline_cache();
    }

    Context::Impl::~Impl() noexcept {
        auto path = stl::filesystem::instance().cache / "pipeline.bin";
        auto size = usize{};
        guard(device->getPipelineCacheData(pipeline_cache.get(), &size, nullptr));
        auto buffer = std::vector<byte>(size);
        guard(device->getPipelineCacheData(pipeline_cache.get(), &size, buffer.data()));
        stl::filesystem::store(path, buffer, std::ios::binary);
    }

    auto Context::Impl::init_instance() noexcept -> void {
        VULKAN_HPP_DEFAULT_DISPATCHER.init();
        auto app = vk::ApplicationInfo{
            .pApplicationName = "metatron",
            .applicationVersion = vk::makeApiVersion(0, 0, 2, 0),
            .pEngineName = "metatron",
            .engineVersion = vk::makeApiVersion(0, 0, 2, 0),
            .apiVersion = vk::makeApiVersion(0, 1, 4, 328),
        };

        auto layers = std::vector<view<char>>{};
        for (auto props: guard(vk::enumerateInstanceLayerProperties())) {
            auto constexpr validation_layer = "VK_LAYER_KHRONOS_validation";
            if (std::string_view{props.layerName} == validation_layer)
                layers.push_back(validation_layer);
        }
        instance = guard(vk::createInstanceUnique({
            .pApplicationInfo = &app,
            .enabledLayerCount = u32(layers.size()),
            .ppEnabledLayerNames = layers.data(),
        }));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());
    }

    auto Context::Impl::init_device() noexcept -> void {
        auto chain = vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceShaderFloat16Int8Features,
            vk::PhysicalDevice8BitStorageFeatures,
            vk::PhysicalDeviceTimelineSemaphoreFeatures,
            vk::PhysicalDeviceSynchronization2Features,
            vk::PhysicalDeviceBufferDeviceAddressFeatures,
            vk::PhysicalDeviceScalarBlockLayoutFeatures,
            vk::PhysicalDeviceUniformBufferStandardLayoutFeatures,
            vk::PhysicalDeviceDescriptorBufferFeaturesEXT,
            vk::PhysicalDeviceDescriptorIndexingFeatures,
            vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
            vk::PhysicalDeviceRayQueryFeaturesKHR
        >{};
        auto& features = chain.get<vk::PhysicalDeviceFeatures2>();
        features.features.shaderInt16 = true;
        features.features.shaderInt64 = true;
        features.features.shaderFloat64 = true;
        features.features.samplerAnisotropy = true;
        auto& precision = chain.get<vk::PhysicalDeviceShaderFloat16Int8Features>();
        precision.shaderInt8 = true;
        precision.shaderFloat16 = true;
        auto& bytestore = chain.get<vk::PhysicalDevice8BitStorageFeatures>();
        bytestore.uniformAndStorageBuffer8BitAccess = true;
        auto& timeline = chain.get<vk::PhysicalDeviceTimelineSemaphoreFeatures>();
        timeline.timelineSemaphore = true;
        auto& sync = chain.get<vk::PhysicalDeviceSynchronization2Features>();
        sync.synchronization2 = true;
        auto& address = chain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
        address.bufferDeviceAddress = true;
        auto& layout = chain.get<vk::PhysicalDeviceScalarBlockLayoutFeatures>();
        layout.scalarBlockLayout = true;
        auto& uniform = chain.get<vk::PhysicalDeviceUniformBufferStandardLayoutFeatures>();
        uniform.uniformBufferStandardLayout = true;
        auto& buffer = chain.get<vk::PhysicalDeviceDescriptorBufferFeaturesEXT>();
        buffer.descriptorBuffer = true;
        auto& bindless = chain.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();
        bindless.shaderSampledImageArrayNonUniformIndexing = true;
        bindless.shaderStorageImageArrayNonUniformIndexing = true;
        bindless.runtimeDescriptorArray = true;
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
            std::vector<view<char>>{
                "VK_KHR_timeline_semaphore",
                "VK_EXT_descriptor_indexing",
            },
            std::vector<view<char>>{
                "VK_KHR_buffer_device_address",
                "VK_KHR_uniform_buffer_standard_layout",
                "VK_KHR_synchronization2",
            },
            std::vector<view<char>>{
                "VK_KHR_maintenance5",
            },
            std::vector<view<char>>{
                "VK_EXT_descriptor_buffer",
                "VK_KHR_deferred_host_operations",
                "VK_KHR_acceleration_structure",
                "VK_KHR_ray_query",
                "VK_KHR_maintenance6",
            },
        });

        for (auto&& physical_device: guard(instance->enumeratePhysicalDevices())) {
            auto families = physical_device.getQueueFamilyProperties2();
            auto render_family = math::maxv<u32>;
            auto render_limit = 0u;
            auto transfer_family = math::maxv<u32>;
            auto transfer_limit = 0u;
            for (auto i = 0; i < families.size(); ++i) {
                auto const& family = families[i];
                auto& props = family.queueFamilyProperties;
                auto flags = props.queueFlags;
                auto graphics = vk::QueueFlags::BitsType::eGraphics;
                auto transfer = vk::QueueFlags::BitsType::eTransfer;
                if (render_family == math::maxv<u32> && flags & graphics) {
                    render_family = i;
                    render_limit = props.queueCount;
                } else if (transfer_family == math::maxv<u32> && flags & transfer) {
                    transfer_family = i;
                    transfer_limit = props.queueCount;
                }
            }
            if (render_family == math::maxv<u32> || transfer_family == math::maxv<u32>)
                stl::abort("no vulkan async transfer queue");

            auto render_priorities = std::vector<f32>(render_limit, 1.f);
            auto transfer_priorities = std::vector<f32>(transfer_limit, 1.f);
            auto queues = std::array<vk::DeviceQueueCreateInfo, 2>{
                vk::DeviceQueueCreateInfo{
                    .queueFamilyIndex = render_family,
                    .queueCount = render_limit,
                    .pQueuePriorities = render_priorities.data(),
                },
                vk::DeviceQueueCreateInfo{
                    .queueFamilyIndex = transfer_family,
                    .queueCount = transfer_limit,
                    .pQueuePriorities = render_priorities.data(),
                },
            };

            auto version = physical_device.getProperties2().properties.apiVersion;
            auto minor = vk::apiVersionMinor(version);
            auto extensions = std::vector<view<char>>{};
            for (auto i = 0; i < versions.size(); ++i)
                if (minor <= vk::apiVersionMinor(versions[i]))
                    std::ranges::copy(
                        required_extensions[i],
                        std::back_inserter(extensions)
                    );

            auto info = vk::DeviceCreateInfo{};
            info.setPEnabledExtensionNames(extensions);
            info.setQueueCreateInfos(queues);
            info.setPNext(chain.get());

            if (auto candidate = physical_device.createDeviceUnique(info);
                candidate.result == vk::Result::eSuccess
            ) {
                device = std::move(candidate.value);
                Queue::Impl::family[u32(Type::render)] = render_family;
                Queue::Impl::count[u32(Type::render)] = render_limit;
                Queue::Impl::family[u32(Type::transfer)] = transfer_family;
                Queue::Impl::count[u32(Type::transfer)] = transfer_limit;

                auto props = vk::StructureChain<
                    vk::PhysicalDeviceProperties2,
                    vk::PhysicalDeviceDescriptorBufferPropertiesEXT,
                    vk::PhysicalDeviceAccelerationStructurePropertiesKHR
                >{};
                physical_device.getProperties2(&props.get());
                device_props = props.get<vk::PhysicalDeviceProperties2>();
                memory_props = physical_device.getMemoryProperties2();
                descriptor_buffer_props = props.get<vk::PhysicalDeviceDescriptorBufferPropertiesEXT>();
                accel_props = props.get<vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();
                break;
            }
        }

        if (!device) stl::abort("no vulkan device meets requirements");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get(), device.get());
    }

    auto Context::Impl::init_memory() noexcept -> void {
        auto props = memory_props.memoryProperties;
        auto device_heap = math::maxv<u32>;
        auto host_heap = math::maxv<u32>;
        for (auto i = 0; i < props.memoryHeapCount; ++i) {
            auto& heap = props.memoryHeaps[i];
            auto local = vk::MemoryHeapFlagBits::eDeviceLocal;
            if (device_heap == math::maxv<u32> && heap.flags & local)
                device_heap = i;
            else if (host_heap == math::maxv<u32> && !(heap.flags & local))
                host_heap = i;
        }
        if (host_heap == math::maxv<u32>) host_heap = device_heap;
        if (device_heap == math::maxv<u32>) stl::abort("no vulkan device local heap");

        auto device_type = math::maxv<u32>;
        auto host_type = math::maxv<u32>;
        for (auto i = 0; i < props.memoryTypeCount; ++i) {
            auto& type = props.memoryTypes[i];
            auto init_type = [&type, i](ref<u32> t, u32 heap, vk::MemoryPropertyFlags flags) {
                if (true
                && t == math::maxv<u32>
                && type.heapIndex == heap
                && type.propertyFlags & flags) t = i;
            };
            init_type(device_type, device_heap, vk::MemoryPropertyFlagBits::eDeviceLocal);
            init_type(host_type, host_heap, vk::MemoryPropertyFlagBits::eHostCoherent);
        }
        if (device_type == math::maxv<u32> || host_type == math::maxv<u32>)
            stl::abort("no vulkan local or visible memory type");
        device_memory = device_type;
        host_memory = host_type;
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
    }
}
