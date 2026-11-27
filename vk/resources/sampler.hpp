#pragma once
#include "../common.hpp"
namespace vk {
namespace sampler {
inline vk::SamplerCreateInfo info(vk::Filter filter,
                                  vk::SamplerAddressMode address = vk::SamplerAddressMode::eClampToEdge,
                                  vk::SamplerMipmapMode mipmap = vk::SamplerMipmapMode::eNearest,
                                  float max_lod = 0.f) {
    vk::SamplerCreateInfo result{};
    result.magFilter = result.minFilter = filter;
    result.addressModeU = result.addressModeV = result.addressModeW = address;
    result.mipmapMode = mipmap;
    result.maxLod = max_lod;
    return result;
}
vk::Sampler create(const vk::SamplerCreateInfo &info);
    void destroy(vk::Sampler handle);
}
}
