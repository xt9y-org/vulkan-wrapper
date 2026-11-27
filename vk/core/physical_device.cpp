#include "../internal.hpp"
#include <cstring>
using vk::internal::require_physical;
namespace vk {
namespace physical_device {
state *get() { static state data; return &data; }
vk::PhysicalDevice adopt(vk::PhysicalDevice handle)
{
    if (!handle) throw std::invalid_argument("null physical device");
    auto &s = *get();
    s.handle = handle;
    s.properties = handle.getProperties();
    s.features = handle.getFeatures();
    s.memory = handle.getMemoryProperties();
    s.queue_families = handle.getQueueFamilyProperties();
    s.subgroups=vk::PhysicalDeviceSubgroupProperties{};
    if(instance::get()->api_version>=VK_API_VERSION_1_1&&s.properties.apiVersion>=VK_API_VERSION_1_1&&
       VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceProperties2){
        vk::PhysicalDeviceProperties2 extended{};
        extended.pNext=&s.subgroups;
        handle.getProperties2(&extended);
    }
    return handle;
}
vk::PhysicalDevice init(uint32_t index)
{
    auto devices = instance::physical_devices();
    if (index >= devices.size()) throw std::out_of_range("physical device index");
    return adopt(devices[index]);
}
vk::PhysicalDevice pick(vk::QueueFlags flags)
{
    auto devices = instance::physical_devices();
    for (auto candidate : devices) {
        for (auto family : candidate.getQueueFamilyProperties()) {
            if (family.queueCount && (family.queueFlags & flags) == flags)
                return adopt(candidate);
        }
    }
    throw std::runtime_error("no compatible physical device");
}
uint32_t find_queue(vk::QueueFlags flags)
{
    auto &families = get()->queue_families;
    for (uint32_t i = 0; i < families.size(); ++i)
        if (families[i].queueCount && (families[i].queueFlags & flags) == flags)
            return i;
    throw std::runtime_error("queue family unavailable");
}
bool supports_extension(const char *name)
{
    for (auto &ext : require_physical().enumerateDeviceExtensionProperties())
        if (std::strcmp(ext.extensionName.data(), name) == 0) return true;
    return false;
}
bool supports_shader_float16_khr()
{
    if(!supports_extension(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)||
       !VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceFeatures2KHR)return false;
    VkPhysicalDeviceShaderFloat16Int8Features half{};
    half.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
    VkPhysicalDeviceFeatures2 features{};
    features.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext=&half;
    VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceFeatures2KHR(
        static_cast<VkPhysicalDevice>(require_physical()),&features);
    return half.shaderFloat16!=VK_FALSE;
}
static bool query_features(void *next)
{
    VkPhysicalDeviceFeatures2 features{};
    features.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext=next;
    auto &dispatch=VULKAN_HPP_DEFAULT_DISPATCHER;
    const auto physical=static_cast<VkPhysicalDevice>(require_physical());
    if(instance::get()->api_version>=VK_API_VERSION_1_1&&dispatch.vkGetPhysicalDeviceFeatures2)
        dispatch.vkGetPhysicalDeviceFeatures2(physical,&features);
    else if(dispatch.vkGetPhysicalDeviceFeatures2KHR)
        dispatch.vkGetPhysicalDeviceFeatures2KHR(physical,&features);
    else return false;
    return true;
}
bool supports_synchronization2_khr()
{
    if(!supports_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME))return false;
    VkPhysicalDeviceSynchronization2Features sync{};
    sync.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    return query_features(&sync) && sync.synchronization2!=VK_FALSE;
}
bool supports_timeline_semaphore_khr()
{
    if(!supports_extension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME))return false;
    VkPhysicalDeviceTimelineSemaphoreFeatures timeline{};
    timeline.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
    return query_features(&timeline) && timeline.timelineSemaphore!=VK_FALSE;
}
void shutdown() { *get() = {}; }
}
}
