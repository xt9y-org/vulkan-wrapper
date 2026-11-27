#include "../internal.hpp"
namespace vk {
namespace assertion {
void that(bool condition, std::string_view message)
{
    if (!condition) {
        log::error(message);
        throw std::runtime_error(std::string(message));
    }
}
}
}
