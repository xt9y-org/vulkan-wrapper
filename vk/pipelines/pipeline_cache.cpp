#include "../internal.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
using vk::internal::require_device;
namespace vk {
namespace pipeline_cache {
state *get() { static state data; return &data; }
vk::PipelineCache init(const vk::PipelineCacheCreateInfo &info)
{
    auto &s = *get();
    if (s.handle) throw std::runtime_error("pipeline cache already initialized");
    s.handle = require_device().createPipelineCache(info);
    return s.handle;
}
std::vector<uint8_t> data()
{
    return require_device().getPipelineCacheData(get()->handle);
}
bool open_file(const std::string &path) noexcept
{
    if(get()->handle||path.empty())return false;
    try {
        std::vector<uint8_t> bytes;
        std::ifstream in(path,std::ios::binary|std::ios::ate);
        if(in){
            const auto n=in.tellg();
            if(n>=std::streamoff(32)&&n<=std::streamoff(64*1024*1024)){
                bytes.resize(static_cast<std::size_t>(n));
                in.seekg(0);
                if(!in.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size())))bytes.clear();
            }
        }
        if(bytes.size()>=32){
            std::array<uint32_t,4> header{};
            std::memcpy(header.data(),bytes.data(),sizeof(header));
            const auto &props=physical_device::get()->properties;
            if(header[0]!=32u||header[1]!=VK_PIPELINE_CACHE_HEADER_VERSION_ONE||
               header[2]!=props.vendorID||header[3]!=props.deviceID||
               std::memcmp(bytes.data()+16,props.pipelineCacheUUID.data(),VK_UUID_SIZE)!=0)bytes.clear();
        }
        vk::PipelineCacheCreateInfo info{};
        info.initialDataSize=bytes.size();info.pInitialData=bytes.empty()?nullptr:bytes.data();
        try{init(info);}catch(...){if(bytes.empty())throw;init();}
        get()->path=path;return true;
    }catch(...){return false;}
}
void save_file() noexcept
{
    const auto &s=*get();
    if(!s.handle||s.path.empty())return;
    try{
        const auto bytes=data();
        if(bytes.empty())return;
        const auto temp=s.path+".tmp";
        {std::ofstream out(temp,std::ios::binary|std::ios::trunc);
         if(!out.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size())))return;
         out.close();if(!out)return;}
        std::error_code error;
        std::filesystem::rename(temp,s.path,error);
        if(error)std::filesystem::remove(temp,error);
    }catch(...){}
}
void shutdown()
{
    auto &s = *get();
    if (s.handle) {save_file();require_device().destroyPipelineCache(s.handle);}
    s = {};
}
}
}
