#include <cbuild.h>

void build(C_Build *b)
{
    C_Target *engine = c_static_library(b, "VulkanWrapper");
    c_sources(engine, "vk/core/*.cpp");
    c_sources(engine, "vk/presentation/*.cpp");
    c_sources(engine, "vk/resources/*.cpp");
    c_sources(engine, "vk/commands/*.cpp");
    c_sources(engine, "vk/descriptors/*.cpp");
    c_sources(engine, "vk/pipelines/*.cpp");
    c_sources(engine, "vk/diagnostics/*.cpp");
    c_sources(engine, "vk/util/*.cpp");
    c_include(engine, ".");
    c_include(engine, "vk");
    c_flag(engine, "-std=c++17");

    C_Dependency *headers = c_git(
        b, "VulkanHeaders", "https://github.com/KhronosGroup/Vulkan-Headers.git", "9fbc2a782ee3f8a4db64e7d991e23da75e893f44");
    c_dep_header_only(headers);
    c_dep_include(headers, "include");
    c_use(engine, headers);

#ifdef _WIN32
    // cbuild quotes the dependency root; join the suffix inside PowerShell
    // so the complete path is passed as one argument to generate.py.
    c_generate(engine, "vk/api/generated.cpp", "@VulkanHeaders@/registry/vk.xml",
        "python vk/tools/generate.py "
        "$(Join-Path @VulkanHeaders@ 'registry/vk.xml') "
        "vk/api/generated.hpp vk/api/generated.cpp --readme readme "
        "--headers $(Join-Path @VulkanHeaders@ 'include') --vendor vk");
#else
    c_generate(engine, "vk/api/generated.cpp", "@VulkanHeaders@/registry/vk.xml",
        "python3 vk/tools/generate.py @VulkanHeaders@/registry/vk.xml "
        "vk/api/generated.hpp vk/api/generated.cpp --readme readme "
        "--headers @VulkanHeaders@/include --vendor vk");
#endif

#ifdef __linux__
    c_link_system(engine, "dl");
#endif
    c_default_target(b, engine);
}
