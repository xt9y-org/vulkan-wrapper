#include "../internal.hpp"
using vk::internal::require_device;
namespace vk {
namespace sync {
namespace {
void require_timeline()
{
    if(!vk::device::timeline_available())throw std::logic_error("Timeline semaphores are not enabled");
}
}
vk::Fence fence(bool signaled)
{
    vk::FenceCreateInfo info{};
    if (signaled) info.flags = vk::FenceCreateFlagBits::eSignaled;
    return require_device().createFence(info);
}
vk::Semaphore semaphore() { return require_device().createSemaphore({}); }
vk::Semaphore timeline(uint64_t initial_value)
{
    require_timeline();
    vk::SemaphoreTypeCreateInfo type{};
    type.semaphoreType = vk::SemaphoreType::eTimeline;
    type.initialValue = initial_value;
    vk::SemaphoreCreateInfo info{};
    info.pNext = &type;
    return require_device().createSemaphore(info);
}
uint64_t counter(vk::Semaphore handle)
{
    require_timeline();
    auto device = static_cast<VkDevice>(require_device());
    const auto &d = VULKAN_HPP_DEFAULT_DISPATCHER;
    uint64_t value = 0;
    const bool core = vk::internal::core_timeline();
    vk::check(core ? d.vkGetSemaphoreCounterValue(device, static_cast<VkSemaphore>(handle), &value) :
                     d.vkGetSemaphoreCounterValueKHR(device, static_cast<VkSemaphore>(handle), &value));
    return value;
}
vk::Result wait(vk::Semaphore handle, uint64_t value, uint64_t timeout)
{
    require_timeline();
    auto device = static_cast<VkDevice>(require_device());
    const auto &d = VULKAN_HPP_DEFAULT_DISPATCHER;
    const VkSemaphore semaphore = static_cast<VkSemaphore>(handle);
    VkSemaphoreWaitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    info.semaphoreCount = 1; info.pSemaphores = &semaphore; info.pValues = &value;
    const bool core = vk::internal::core_timeline();
    const auto result = core ? d.vkWaitSemaphores(device, &info, timeout) : d.vkWaitSemaphoresKHR(device, &info, timeout);
    if (result != VK_TIMEOUT) vk::check(result);
    return static_cast<vk::Result>(result);
}
void signal(vk::Semaphore handle, uint64_t value)
{
    require_timeline();
    auto device = static_cast<VkDevice>(require_device());
    const auto &d = VULKAN_HPP_DEFAULT_DISPATCHER;
    VkSemaphoreSignalInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    info.semaphore = static_cast<VkSemaphore>(handle); info.value = value;
    const bool core = vk::internal::core_timeline();
    vk::check(core ? d.vkSignalSemaphore(device, &info) : d.vkSignalSemaphoreKHR(device, &info));
}
void destroy(vk::Fence handle)
{
    if (handle) require_device().destroyFence(handle);
}
void destroy(vk::Semaphore handle)
{
    if (handle) require_device().destroySemaphore(handle);
}
}
}
