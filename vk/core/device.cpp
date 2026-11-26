#include "../internal.hpp"
#include "../resources/memory.hpp"
using vk::internal::require_device;
using vk::internal::require_instance;
using vk::internal::require_physical;
namespace vk {
namespace device {
state *get() { static state data; return &data; }
vk::Device init(const vk::DeviceCreateInfo &info)
{
    if (get()->handle) throw std::runtime_error("device already initialized");
    auto handle = require_physical().createDevice(info);
    adopt(handle, true);
    for(auto *next=static_cast<const VkBaseInStructure*>(info.pNext);next;next=next->pNext){
        if(next->sType==VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES)
            get()->shader_float16=reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features*>(next)->shaderFloat16!=VK_FALSE;
        else if(next->sType==VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES)
            get()->synchronization2=reinterpret_cast<const VkPhysicalDeviceSynchronization2Features*>(next)->synchronization2!=VK_FALSE;
        else if(next->sType==VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES)
            get()->synchronization2=reinterpret_cast<const VkPhysicalDeviceVulkan13Features*>(next)->synchronization2!=VK_FALSE;
        else if(next->sType==VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES)
            get()->timeline_semaphore=reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures*>(next)->timelineSemaphore!=VK_FALSE;
        else if(next->sType==VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES){
            const auto *features=reinterpret_cast<const VkPhysicalDeviceVulkan12Features*>(next);
            get()->shader_float16=features->shaderFloat16!=VK_FALSE;
            get()->timeline_semaphore=features->timelineSemaphore!=VK_FALSE;
        }
    }
    return handle;
}
void adopt(vk::Device handle, bool owned)
{
    if (!handle) throw std::invalid_argument("null logical device");
    if (get()->handle) throw std::runtime_error("device already initialized");
    get()->handle = handle;
    get()->owned = owned;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(handle);
}
void idle() { require_device().waitIdle(); }
void shutdown()
{
    auto &s = *get();
    if (s.handle) {pipeline_cache::shutdown();memory::shutdown();}
    if (s.owned && s.handle) s.handle.destroy();
    s = {};
}
}
}
