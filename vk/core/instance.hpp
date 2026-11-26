#pragma once
#include "../common.hpp"
namespace vk {
namespace instance {
struct state {
        vk::Instance handle{};
        vk::Instance *api() { return &handle; }
        const vk::Instance *api() const { return &handle; }
        bool owned = false;
        uint32_t api_version = VK_API_VERSION_1_0;
    };
    state *get();
    vk::Instance init(const vk::InstanceCreateInfo &info);
    void adopt(vk::Instance handle, bool owned = false);
    uint32_t loader_api_version();
    std::vector<vk::PhysicalDevice> physical_devices();
    std::vector<vk::ExtensionProperties> extensions(const char *layer = nullptr);
    std::vector<vk::LayerProperties> layers();
    void shutdown();
}
}
