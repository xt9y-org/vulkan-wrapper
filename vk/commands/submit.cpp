#include "../internal.hpp"
namespace vk::submit {
void queue(vk::Queue q, const std::vector<vk::SubmitInfo> &submits, vk::Fence f) {
    q.submit(submits, f);
}
void queue2(vk::Queue q, const std::vector<vk::SubmitInfo2> &submits, vk::Fence f) {
    if (!vk::device::synchronization2_available())
        throw std::logic_error("Synchronization2 is not enabled");
    const auto &d = VULKAN_HPP_DEFAULT_DISPATCHER;
    if (vk::device::core_api(VK_API_VERSION_1_3) && d.vkQueueSubmit2 && d.vkCmdPipelineBarrier2)
        q.submit2(submits, f);
    else
        q.submit2KHR(submits, f);
}
void wait(vk::Queue q) { q.waitIdle(); }
vk::Result fence(vk::Fence f, uint64_t timeout) {
    return vk::internal::require_device().waitForFences(f, true, timeout);
}
void reset(vk::Fence f) { vk::internal::require_device().resetFences(f); }
vk::Result present(vk::Queue q, const vk::PresentInfoKHR &info) {
    return q.presentKHR(info);
}
}
