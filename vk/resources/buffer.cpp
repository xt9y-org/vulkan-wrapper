#include "../internal.hpp"
#include <cstring>
using vk::internal::require_device;
using vk::internal::require_instance;
using vk::internal::require_physical;
namespace vk {
namespace buffer {
resource create(vk::DeviceSize size, vk::BufferUsageFlags usage,
                vk::MemoryPropertyFlags properties)
{
    auto d = require_device();
    vk::BufferCreateInfo info{};
    info.size = size;
    info.usage = usage;
    info.sharingMode = vk::SharingMode::eExclusive;
    resource r{};
    r.handle = d.createBuffer(info);
    r.size = size;
    r.properties = properties;
    try {
        const auto allocation = memory::allocate_buffer(
            d.getBufferMemoryRequirements(r.handle), properties);
        r.memory = allocation.handle;
        r.memory_offset = allocation.offset;
        r.memory_allocation_size = allocation.size;
        r.pooled = allocation.pooled;
        d.bindBufferMemory(r.handle, r.memory, r.memory_offset);
    } catch (...) {
        if (r.memory)
            memory::free_buffer({r.memory, r.memory_offset,
                                 r.memory_allocation_size, r.pooled});
        d.destroyBuffer(r.handle);
        throw;
    }
    return r;
}
void destroy(resource &r)
{
    auto d = require_device();
    if (r.mapped) memory::unmap_buffer(
        {r.memory, r.memory_offset, r.memory_allocation_size, r.pooled});
    if (r.handle) d.destroyBuffer(r.handle);
    if (r.memory) memory::free_buffer(
        {r.memory, r.memory_offset, r.memory_allocation_size, r.pooled});
    r = {};
}
void *map(resource &r)
{
    if (!r.mapped) r.mapped = memory::map_buffer(
        {r.memory, r.memory_offset, r.memory_allocation_size, r.pooled});
    return r.mapped;
}
void unmap(resource &r)
{
    if (r.mapped) memory::unmap_buffer(
        {r.memory, r.memory_offset, r.memory_allocation_size, r.pooled});
    r.mapped = nullptr;
}
void flush(const resource &r)
{
    if (r.properties & vk::MemoryPropertyFlagBits::eHostCoherent) return;
    memory::flush_buffer(
        {r.memory, r.memory_offset, r.memory_allocation_size, r.pooled});
}
void write(resource &r, const void *data, size_t bytes, size_t offset)
{
    if (!data || offset > r.size || bytes > r.size - offset)
        throw std::out_of_range("buffer write exceeds buffer size");
    std::memcpy(static_cast<char *>(map(r)) + offset, data, bytes);
    flush(r);
}
vk::DescriptorBufferInfo descriptor(const resource &r,
                                     vk::DeviceSize offset, vk::DeviceSize range)
{
    vk::DescriptorBufferInfo info{};
    info.buffer = r.handle;
    info.offset = offset;
    info.range = range;
    return info;
}
}
}
