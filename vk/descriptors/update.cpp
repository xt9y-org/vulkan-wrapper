#include "../internal.hpp"
namespace vk::descriptor_update {
void apply(const vk::ArrayProxy<const vk::WriteDescriptorSet> &writes,
           const vk::ArrayProxy<const vk::CopyDescriptorSet> &copies) {
    vk::internal::require_device().updateDescriptorSets(writes, copies);
}
void write(const vk::ArrayProxy<const vk::WriteDescriptorSet> &writes) { apply(writes, {}); }
void copy(const vk::ArrayProxy<const vk::CopyDescriptorSet> &copies) { apply({}, copies); }
}
