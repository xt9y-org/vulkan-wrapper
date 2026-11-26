#pragma once
#include "../common.hpp"
namespace vk {
namespace sync {
vk::Fence fence(bool signaled = false);
    vk::Semaphore semaphore();
    vk::Semaphore timeline(uint64_t initial_value = 0);
    uint64_t counter(vk::Semaphore handle);
    vk::Result wait(vk::Semaphore handle, uint64_t value, uint64_t timeout = UINT64_MAX);
    void signal(vk::Semaphore handle, uint64_t value);
    void destroy(vk::Fence handle);
    void destroy(vk::Semaphore handle);
}
}
