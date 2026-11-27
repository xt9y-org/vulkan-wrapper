#include "../internal.hpp"
#include <memory>
#include <cstdlib>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
namespace { std::unique_ptr<vk::detail::DynamicLoader> dynamic_loader; }
namespace vk {
namespace loader {
state *get() { static state data; return &data; }
bool init(PFN_vkGetInstanceProcAddr proc)
{
#if defined(__APPLE__)
    // MoltenVK reads this setting when the driver initializes. Respect an
    // explicit user override and let the driver handle unsupported devices.
    if(!std::getenv("MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS"))
        setenv("MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS","1",0);
#endif
    if (proc == nullptr) {
        dynamic_loader = std::make_unique<vk::detail::DynamicLoader>();
        proc = dynamic_loader->getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    }
    if (!proc) {
        get()->initialized = false;
        get()->get_instance_proc_addr = nullptr;
        return false;
    }
    VULKAN_HPP_DEFAULT_DISPATCHER.init(proc);
    get()->get_instance_proc_addr = proc;
    get()->initialized = true;
    return true;
}
void shutdown()
{
    get()->initialized = false;
    get()->get_instance_proc_addr = nullptr;
    dynamic_loader.reset();
}
}
}
