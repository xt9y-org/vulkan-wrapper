#pragma once
#include "../common.hpp"
namespace vk {
namespace memory {
struct buffer_allocation {
    vk::DeviceMemory handle{};
    vk::DeviceSize offset = 0;
    vk::DeviceSize size = 0;
    bool pooled = false;
};

uint32_t type_index(uint32_t bits, vk::MemoryPropertyFlags flags);
    vk::DeviceMemory allocate(const vk::MemoryRequirements &requirements,
                              vk::MemoryPropertyFlags flags);
    buffer_allocation allocate_buffer(const vk::MemoryRequirements &requirements,
                                      vk::MemoryPropertyFlags flags);
    void free_buffer(const buffer_allocation &allocation);
    void *map_buffer(const buffer_allocation &allocation);
    void unmap_buffer(const buffer_allocation &allocation);
    void flush_buffer(const buffer_allocation &allocation);
    void shutdown();
    void free(vk::DeviceMemory handle);
}
}
