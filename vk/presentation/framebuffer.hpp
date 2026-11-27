#pragma once
#include "../common.hpp"
#include <span>
namespace vk::framebuffer {
vk::Framebuffer create(const vk::FramebufferCreateInfo &info);
vk::Framebuffer create(vk::RenderPass pass, std::span<const vk::ImageView> views,
                       vk::Extent2D extent, uint32_t layers = 1);
void destroy(vk::Framebuffer handle);
}
