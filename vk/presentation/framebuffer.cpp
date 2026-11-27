#include "../internal.hpp"
namespace vk::framebuffer {
vk::Framebuffer create(const vk::FramebufferCreateInfo &info) {
    return vk::internal::require_device().createFramebuffer(info);
}
vk::Framebuffer create(vk::RenderPass pass, std::span<const vk::ImageView> views,
                       vk::Extent2D extent, uint32_t layers) {
    vk::FramebufferCreateInfo info{};
    info.renderPass = pass;
    info.attachmentCount = uint32_t(views.size());
    info.pAttachments = views.data();
    info.width = extent.width;
    info.height = extent.height;
    info.layers = layers;
    return create(info);
}
void destroy(vk::Framebuffer handle) {
    if (handle) vk::internal::require_device().destroyFramebuffer(handle);
}
}
