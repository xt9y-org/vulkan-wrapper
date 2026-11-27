#include "../internal.hpp"
using vk::internal::require_device;
namespace vk {
namespace descriptor {
state *get() { static state data; return &data; }
vk::DescriptorPool create(const vk::DescriptorPoolCreateInfo &info)
{return require_device().createDescriptorPool(info);}
void destroy(vk::DescriptorPool pool)
{if(pool)require_device().destroyDescriptorPool(pool);}
vk::DescriptorPool init(const vk::DescriptorPoolCreateInfo &info)
{
    auto &s = *get();
    if (s.pool) throw std::runtime_error("descriptor pool already initialized");
    s.pool = create(info);
    return s.pool;
}
std::vector<vk::DescriptorSet> allocate(vk::DescriptorPool pool,const vk::ArrayProxy<const vk::DescriptorSetLayout> &layouts)
{
    vk::DescriptorSetAllocateInfo info{};
    info.descriptorPool = pool;
    info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    info.pSetLayouts = layouts.data();
    return require_device().allocateDescriptorSets(info);
}
std::vector<vk::DescriptorSet> allocate(const vk::ArrayProxy<const vk::DescriptorSetLayout> &layouts)
{return allocate(get()->pool,layouts);}
void reset()
{
    require_device().resetDescriptorPool(get()->pool);
}
void shutdown()
{
    auto &s = *get();
    destroy(s.pool);
    s = {};
}
}
}
