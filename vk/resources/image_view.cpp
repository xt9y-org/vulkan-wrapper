#include "../internal.hpp"
using vk::internal::require_device;
namespace vk {
namespace image_view {
vk::ImageView create(const vk::ImageViewCreateInfo &info)
{
    return require_device().createImageView(info);
}
void destroy(vk::ImageView handle)
{
    if (handle) require_device().destroyImageView(handle);
}
}
}
