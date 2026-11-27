#include "../internal.hpp"
using vk::internal::require_instance;
namespace vk {
namespace instance {
state *get() { static state data; return &data; }
vk::Instance init(const vk::InstanceCreateInfo &info)
{
    if (get()->handle) throw std::runtime_error("vk::instance already initialized");
    if (!loader::get()->initialized && !loader::init())
        throw std::runtime_error("Vulkan loader unavailable");
    auto handle = vk::createInstance(info);
    adopt(handle, true);
    if(info.pApplicationInfo&&info.pApplicationInfo->apiVersion)
        get()->api_version=info.pApplicationInfo->apiVersion;
    return handle;
}
void adopt(vk::Instance handle, bool owned)
{
    if (!loader::get()->initialized) throw std::runtime_error("vk::loader not initialized");
    if (get()->handle) throw std::runtime_error("vk::instance already initialized");
    get()->handle = handle;
    get()->owned = owned;
    if (handle) VULKAN_HPP_DEFAULT_DISPATCHER.init(handle);
}
uint32_t loader_api_version()
{
    uint32_t version=VK_API_VERSION_1_0;
    auto query=VULKAN_HPP_DEFAULT_DISPATCHER.vkEnumerateInstanceVersion;
    if(query&&query(&version)!=VK_SUCCESS)return VK_API_VERSION_1_0;
    return version;
}
std::vector<vk::PhysicalDevice> physical_devices()
{
    return require_instance().enumeratePhysicalDevices();
}
std::vector<vk::ExtensionProperties> extensions(const char *layer)
{
    if (!loader::get()->initialized && !loader::init())
        throw std::runtime_error("Vulkan loader unavailable");
    if (layer) {
        const std::string layer_name(layer);
        return vk::enumerateInstanceExtensionProperties(layer_name);
    }
    return vk::enumerateInstanceExtensionProperties(nullptr);
}
std::vector<vk::LayerProperties> layers()
{
    if (!loader::get()->initialized && !loader::init())
        throw std::runtime_error("Vulkan loader unavailable");
    return vk::enumerateInstanceLayerProperties();
}
void shutdown()
{
    auto &s = *get();
    if (s.owned && s.handle) s.handle.destroy();
    s = {};
}
}
}
