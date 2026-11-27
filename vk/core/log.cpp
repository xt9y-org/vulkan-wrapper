#include "../internal.hpp"
#include <cstdio>
namespace { vk::log::callback log_callback = nullptr; }
namespace vk {
namespace log {
void set_callback(callback fn) { log_callback = fn; }
void write(level severity, std::string_view message)
{
    if (log_callback) {
        log_callback(severity, message);
        return;
    }
    const char *names[] = {"debug", "info", "warning", "error"};
    std::fprintf(stderr, "[vk:%s] %.*s\n", names[static_cast<int>(severity)],
                 static_cast<int>(message.size()), message.data());
}
void debug(std::string_view message) { write(level::debug, message); }
void info(std::string_view message) { write(level::info, message); }
void warning(std::string_view message) { write(level::warning, message); }
void error(std::string_view message) { write(level::error, message); }
}
}
