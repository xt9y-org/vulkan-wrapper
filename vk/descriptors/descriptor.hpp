#pragma once
#include "../common.hpp"

namespace vk::descriptor {
vk::DescriptorPool create(const vk::DescriptorPoolCreateInfo &info);
void destroy(vk::DescriptorPool pool);
std::vector<vk::DescriptorSet> allocate(vk::DescriptorPool pool,
    const vk::ArrayProxy<const vk::DescriptorSetLayout> &layouts);
}
