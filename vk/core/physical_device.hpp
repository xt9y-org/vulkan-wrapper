#pragma once
#include "../common.hpp"
namespace vk {
namespace physical_device {
struct state {
        vk::PhysicalDevice handle{};
        vk::PhysicalDevice *api() { return &handle; }
        const vk::PhysicalDevice *api() const { return &handle; }
        vk::PhysicalDeviceProperties properties{};
        vk::PhysicalDeviceFeatures features{};
        vk::PhysicalDeviceMemoryProperties memory{};
        std::vector<vk::QueueFamilyProperties> queue_families;
        vk::PhysicalDeviceSubgroupProperties subgroups{};
        bool compute_subgroups(vk::SubgroupFeatureFlags required) const
        {
            return subgroups.subgroupSize>0&&(subgroups.supportedStages&vk::ShaderStageFlagBits::eCompute)&&
                (subgroups.supportedOperations&required)==required;
        }
    };
    state *get();
    vk::PhysicalDevice init(uint32_t index = 0);
    vk::PhysicalDevice adopt(vk::PhysicalDevice handle);
    vk::PhysicalDevice pick(vk::QueueFlags flags = vk::QueueFlagBits::eGraphics);
    uint32_t find_queue(vk::QueueFlags flags);
    bool supports_extension(const char *name);
    // For Vulkan 1.0 instances with KHR_get_physical_device_properties2 enabled.
    bool supports_shader_float16_khr();
    bool supports_synchronization2_khr();
    bool supports_timeline_semaphore_khr();
    void shutdown();
}
}
