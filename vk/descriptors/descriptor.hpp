#pragma once
#include "../common.hpp"
namespace vk {
namespace descriptor {
struct state {
        vk::DescriptorPool pool{};
    };
    state *get();
    vk::DescriptorPool create(const vk::DescriptorPoolCreateInfo &info);
    void destroy(vk::DescriptorPool pool);
    std::vector<vk::DescriptorSet> allocate(vk::DescriptorPool pool,const vk::ArrayProxy<const vk::DescriptorSetLayout> &layouts);
    vk::DescriptorPool init(const vk::DescriptorPoolCreateInfo &info);
    std::vector<vk::DescriptorSet> allocate(const vk::ArrayProxy<const vk::DescriptorSetLayout> &layouts);
    void reset();
    void shutdown();
}
}
