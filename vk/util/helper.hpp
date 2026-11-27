#pragma once
#include "../common.hpp"
#include <utility>
namespace vk {
namespace util {
// Unique ownership of a device handle, independent of its allocation policy.
// The owner must retire it after the final GPU use, before device teardown.
template<class Handle,void (*Destroy)(Handle)>
class owned_handle {
    Handle handle_{};
public:
    owned_handle() = default;
    explicit owned_handle(Handle handle) noexcept : handle_(handle) {}
    owned_handle(const owned_handle &) = delete;
    owned_handle &operator=(const owned_handle &) = delete;
    owned_handle(owned_handle &&other) noexcept : handle_(other.release()) {}
    owned_handle &operator=(owned_handle &&other) noexcept {
        if(this!=&other)reset(other.release());
        return *this;
    }
    ~owned_handle(){reset();}
    Handle get() const noexcept{return handle_;}
    explicit operator bool() const noexcept{return bool(handle_);}
    Handle release() noexcept{return std::exchange(handle_,Handle{});}
    void reset(Handle next={}) noexcept {
        if(handle_==next)return;
        const auto old=std::exchange(handle_,next);
        if(old)try{Destroy(old);}catch(...){}
    }
};
vk::Extent2D extent(uint32_t width, uint32_t height);
    uint32_t mip_levels(uint32_t width, uint32_t height);
    vk::DeviceSize aligned(vk::DeviceSize value, vk::DeviceSize alignment);
    std::string result_string(vk::Result result);
}
}
