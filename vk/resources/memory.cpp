#include "../internal.hpp"

#include <limits>
#include <mutex>

using vk::internal::require_device;

namespace vk {
namespace memory {
namespace {

constexpr vk::DeviceSize small_buffer_limit = 1024 * 1024;
constexpr vk::DeviceSize buffer_pool_size = 16 * 1024 * 1024;

vk::DeviceMemory allocate_type(vk::DeviceSize size,uint32_t type)
{
    vk::MemoryAllocateInfo info{};
    info.allocationSize=size;info.memoryTypeIndex=type;
    return require_device().allocateMemory(info);
}

struct free_range {
    vk::DeviceSize offset;
    vk::DeviceSize size;
};

struct buffer_block {
    vk::DeviceMemory memory{};
    vk::DeviceSize size = 0;
    uint32_t type = 0;
    size_t live_allocations = 0;
    void *mapped = nullptr;
    std::vector<free_range> free_ranges;
};

std::mutex buffer_pool_mutex;
std::vector<buffer_block> buffer_blocks;

vk::DeviceSize align_up(vk::DeviceSize value, vk::DeviceSize alignment)
{
    if (!alignment) return value;
    const auto remainder = value % alignment;
    if (!remainder) return value;
    const auto addition = alignment - remainder;
    if (value > std::numeric_limits<vk::DeviceSize>::max() - addition)
        throw std::overflow_error("Vulkan buffer allocation size overflow");
    return value + addition;
}

auto find_block(vk::DeviceMemory memory)
{
    auto block = std::find_if(buffer_blocks.begin(), buffer_blocks.end(),
                        [memory](const buffer_block &block) {
                            return block.memory == memory;
                        });
    if (block == buffer_blocks.end())
        throw std::runtime_error("Vulkan buffer pool allocation was not found");
    return block;
}

bool allocate_range(buffer_block &block, vk::DeviceSize size,
                    vk::DeviceSize alignment, vk::DeviceSize &offset)
{
    for (auto range = block.free_ranges.begin(); range != block.free_ranges.end(); ++range)
    {
        const auto range_end = range->offset + range->size;
        const auto start = align_up(range->offset, alignment);
        if (start > range_end || size > range_end - start) continue;

        const auto old_offset = range->offset;
        const auto old_end = range_end;
        const auto allocation_end = start + size;
        auto insertion = block.free_ranges.erase(range);
        if (start > old_offset)
            insertion = block.free_ranges.insert(
                insertion, free_range{old_offset, start - old_offset}) + 1;
        if (allocation_end < old_end)
            block.free_ranges.insert(
                insertion, free_range{allocation_end, old_end - allocation_end});

        ++block.live_allocations;
        offset = start;
        return true;
    }
    return false;
}

void release_range(buffer_block &block, vk::DeviceSize offset,
                   vk::DeviceSize size)
{
    auto position = std::lower_bound(
        block.free_ranges.begin(), block.free_ranges.end(), offset,
        [](const free_range &range, vk::DeviceSize value) {
            return range.offset < value;
        });
    position = block.free_ranges.insert(position, free_range{offset, size});
    if (position != block.free_ranges.begin())
    {
        auto previous = position - 1;
        if (previous->offset + previous->size == position->offset)
        {
            previous->size += position->size;
            position = block.free_ranges.erase(position);
            position = previous;
        }
    }
    auto next = position + 1;
    if (next != block.free_ranges.end() &&
        position->offset + position->size == next->offset)
    {
        position->size += next->size;
        block.free_ranges.erase(next);
    }
    if (block.live_allocations) --block.live_allocations;
}

} // namespace

uint32_t type_index(uint32_t bits, vk::MemoryPropertyFlags flags)
{
    auto &p = physical_device::get()->memory;
    if (!physical_device::get()->handle) throw std::runtime_error("physical device not initialized");
    for (uint32_t i = 0; i < p.memoryTypeCount; ++i)
        if ((bits & (1u << i)) && (p.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    throw std::runtime_error("compatible Vulkan memory type not found");
}

vk::DeviceMemory allocate(const vk::MemoryRequirements &requirements,
                          vk::MemoryPropertyFlags flags)
{
    return allocate_type(requirements.size,type_index(requirements.memoryTypeBits,flags));
}

buffer_allocation allocate_buffer(const vk::MemoryRequirements &requirements,
                                  vk::MemoryPropertyFlags flags)
{
    const auto type = type_index(requirements.memoryTypeBits, flags);
    if (requirements.size > small_buffer_limit)
        return {allocate_type(requirements.size,type),0,requirements.size,false};

    const auto atom_size = std::max<vk::DeviceSize>(
        1, physical_device::get()->properties.limits.nonCoherentAtomSize);
    const auto alignment = std::max(requirements.alignment, atom_size);
    const auto allocation_size = align_up(requirements.size, atom_size);

    std::lock_guard<std::mutex> lock(buffer_pool_mutex);
    for (auto &block : buffer_blocks)
    {
        if (block.type != type) continue;
        vk::DeviceSize offset = 0;
        if (allocate_range(block, allocation_size, alignment, offset))
            return {block.memory, offset, allocation_size, true};
    }

    const auto block_size = align_up(
        std::max(buffer_pool_size, allocation_size), atom_size);
    buffer_block block{};
    block.size = block_size;
    block.type = type;
    block.memory=allocate_type(block_size,type);
    try
    {
        block.free_ranges.push_back({0, block_size});
        buffer_blocks.push_back(std::move(block));
    }
    catch (...)
    {
        require_device().freeMemory(block.memory);
        throw;
    }

    vk::DeviceSize offset = 0;
    if (!allocate_range(buffer_blocks.back(), allocation_size, alignment, offset))
        throw std::runtime_error("new Vulkan buffer pool block cannot fit allocation");
    return {buffer_blocks.back().memory, offset, allocation_size, true};
}

void free_buffer(const buffer_allocation &allocation)
{
    if (!allocation.handle) return;
    if (!allocation.pooled)
    {
        require_device().freeMemory(allocation.handle);
        return;
    }

    std::lock_guard<std::mutex> lock(buffer_pool_mutex);
    auto block = find_block(allocation.handle);
    release_range(*block, allocation.offset, allocation.size);

    if (block->live_allocations != 0) return;
    const auto other_empty = std::any_of(
        buffer_blocks.begin(), buffer_blocks.end(),
        [block](const buffer_block &candidate) {
            return &candidate != &*block && candidate.type == block->type &&
                   candidate.live_allocations == 0;
        });
    if (other_empty)
    {
        if (block->mapped) require_device().unmapMemory(block->memory);
        require_device().freeMemory(block->memory);
        buffer_blocks.erase(block);
    }
}

void *map_buffer(const buffer_allocation &allocation)
{
    if (!allocation.pooled)
        return static_cast<char *>(require_device().mapMemory(
                   allocation.handle, 0, VK_WHOLE_SIZE)) + allocation.offset;

    std::lock_guard<std::mutex> lock(buffer_pool_mutex);
    auto block = find_block(allocation.handle);
    if (!block->mapped)
        block->mapped = require_device().mapMemory(
            block->memory, 0, VK_WHOLE_SIZE);
    return static_cast<char *>(block->mapped) + allocation.offset;
}

void unmap_buffer(const buffer_allocation &allocation)
{
    if (allocation.handle && !allocation.pooled)
        require_device().unmapMemory(allocation.handle);
}

void flush_buffer(const buffer_allocation &allocation)
{
    vk::MappedMemoryRange range{};
    range.memory = allocation.handle;
    range.offset = allocation.pooled ? allocation.offset : 0;
    range.size = allocation.pooled ? allocation.size : VK_WHOLE_SIZE;
    require_device().flushMappedMemoryRanges(range);
}

void shutdown()
{
    std::lock_guard<std::mutex> lock(buffer_pool_mutex);
    if (!device::get()->handle)
    {
        buffer_blocks.clear();
        return;
    }
    auto d = require_device();
    for (auto &block : buffer_blocks)
    {
        if (block.mapped) d.unmapMemory(block.memory);
        if (block.memory) d.freeMemory(block.memory);
    }
    buffer_blocks.clear();
}

void free(vk::DeviceMemory handle)
{
    if (handle) require_device().freeMemory(handle);
}
}
}
