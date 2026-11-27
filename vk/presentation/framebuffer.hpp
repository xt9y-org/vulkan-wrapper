#pragma once
#include "../common.hpp"
namespace vk::framebuffer {
vk::Framebuffer create(const vk::FramebufferCreateInfo &info);
vk::Framebuffer create(vk::RenderPass pass, const vk::ImageView *views, uint32_t count,
                       vk::Extent2D extent, uint32_t layers = 1);
void destroy(vk::Framebuffer handle);
}
