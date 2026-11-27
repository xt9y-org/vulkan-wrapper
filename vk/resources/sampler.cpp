#include "../internal.hpp"
using vk::internal::require_device;
namespace vk {
namespace sampler {
vk::Sampler create(const vk::SamplerCreateInfo &info)
{
    return require_device().createSampler(info);
}
void destroy(vk::Sampler handle)
{
    if (handle) require_device().destroySampler(handle);
}
}
}
