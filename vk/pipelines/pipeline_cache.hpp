#pragma once
#include "../common.hpp"
namespace vk {
namespace pipeline_cache {
struct state {
        vk::PipelineCache handle{};
        std::string path;
    };
    state *get();
    vk::PipelineCache init(const vk::PipelineCacheCreateInfo &info = {});
    std::vector<uint8_t> data();
    // Best effort; invalid/incompatible files are ignored. No cache is required.
    bool open_file(const std::string &path) noexcept;
    void save_file() noexcept;
    void shutdown();
}
}
