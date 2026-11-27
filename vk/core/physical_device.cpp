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
static uint32_t queue_index(const std::vector<vk::QueueFamilyProperties> &families,vk::QueueFlags flags)
{
    for(uint32_t i=0;i<families.size();++i)
        if(families[i].queueCount&&(families[i].queueFlags&flags)==flags)return i;
    return UINT32_MAX;
}
vk::PhysicalDevice pick(vk::QueueFlags flags)
{
    for(auto candidate:instance::physical_devices())
        if(queue_index(candidate.getQueueFamilyProperties(),flags)!=UINT32_MAX)return adopt(candidate);
    throw std::runtime_error("no compatible physical device");
}
uint32_t find_queue(vk::QueueFlags flags)
{
    const auto index=queue_index(get()->queue_families,flags);
    if(index==UINT32_MAX)throw std::runtime_error("queue family unavailable");
    return index;
}
bool supports_extension(const char *name)
{
    for (auto &ext : require_physical().enumerateDeviceExtensionProperties())
        if (std::strcmp(ext.extensionName.data(), name) == 0) return true;
    return false;
}
static bool query_features(void *next,bool khr_only=false)
{
    VkPhysicalDeviceFeatures2 features{};
    features.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext=next;
    auto &dispatch=VULKAN_HPP_DEFAULT_DISPATCHER;
    const auto physical=static_cast<VkPhysicalDevice>(require_physical());
    if(!khr_only&&instance::get()->api_version>=VK_API_VERSION_1_1&&dispatch.vkGetPhysicalDeviceFeatures2)
        dispatch.vkGetPhysicalDeviceFeatures2(physical,&features);
    else if(dispatch.vkGetPhysicalDeviceFeatures2KHR)
        dispatch.vkGetPhysicalDeviceFeatures2KHR(physical,&features);
    else return false;
    return true;
}
template<class Feature>
static bool supports_feature(const char *extension,VkStructureType type,VkBool32 Feature::*member,bool khr_only=false)
{
    if(!supports_extension(extension))return false;
    Feature feature{};
    feature.sType=type;
    return query_features(&feature,khr_only)&&(feature.*member)!=VK_FALSE;
}
bool supports_shader_float16_khr()
{
    return supports_feature(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES,
        &VkPhysicalDeviceShaderFloat16Int8Features::shaderFloat16,true);
}
bool supports_synchronization2_khr()
{
    return supports_feature(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        &VkPhysicalDeviceSynchronization2Features::synchronization2);
}
bool supports_timeline_semaphore_khr()
{
    return supports_feature(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
        &VkPhysicalDeviceTimelineSemaphoreFeatures::timelineSemaphore);
}
void shutdown() { *get() = {}; }
}
}
