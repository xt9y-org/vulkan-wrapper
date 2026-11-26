#pragma once
#include "../common.hpp"
namespace vk {
namespace device {
struct state {
        vk::Device handle{};
        vk::Device *api() { return &handle; }
        const vk::Device *api() const { return &handle; }
        bool owned = false;
        bool shader_float16 = false;
        bool synchronization2 = false;
        bool timeline_semaphore = false;
    };
    state *get();
    vk::Device init(const vk::DeviceCreateInfo &info);
    void adopt(vk::Device handle, bool owned = false);
    void idle();
    void shutdown();
}
}
