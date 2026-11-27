#include "../internal.hpp"
using vk::internal::require_device;
namespace vk::descriptor {
vk::DescriptorPool create(const vk::DescriptorPoolCreateInfo &info)
{return require_device().createDescriptorPool(info);}
void destroy(vk::DescriptorPool pool)
{if(pool)require_device().destroyDescriptorPool(pool);}
std::vector<vk::DescriptorSet> allocate(vk::DescriptorPool pool,const vk::ArrayProxy<const vk::DescriptorSetLayout> &layouts)
{
    vk::DescriptorSetAllocateInfo info{};
    info.descriptorPool = pool;
    info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    info.pSetLayouts = layouts.data();
    return require_device().allocateDescriptorSets(info);
}
}
