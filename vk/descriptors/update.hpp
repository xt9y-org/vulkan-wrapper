#pragma once
#include "../common.hpp"
namespace vk::descriptor_update {
inline vk::WriteDescriptorSet binding(vk::DescriptorSet set, uint32_t slot,
                                      vk::DescriptorType type, uint32_t element = 0) {
    vk::WriteDescriptorSet result{};
    result.dstSet = set;
    result.dstBinding = slot;
    result.dstArrayElement = element;
    result.descriptorType = type;
    result.descriptorCount = 1;
    return result;
}
inline vk::WriteDescriptorSet binding(vk::DescriptorSet set,uint32_t slot,
                                     vk::DescriptorType type,const vk::DescriptorBufferInfo &info,uint32_t element=0)
{
    auto result=binding(set,slot,type,element);
    result.pBufferInfo=&info;return result;
}
inline vk::WriteDescriptorSet binding(vk::DescriptorSet set,uint32_t slot,
                                     vk::DescriptorType type,const vk::DescriptorImageInfo &info,uint32_t element=0)
{
    auto result=binding(set,slot,type,element);
    result.pImageInfo=&info;return result;
}
void write(const std::vector<vk::WriteDescriptorSet> &writes);
void copy(const std::vector<vk::CopyDescriptorSet> &copies);
void apply(const std::vector<vk::WriteDescriptorSet> &writes,
           const std::vector<vk::CopyDescriptorSet> &copies);
}
