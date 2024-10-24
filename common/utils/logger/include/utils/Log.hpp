#pragma once

#include <print>
#include <string_view>
#include <utility>

enum class LogLevel {
    fatal,
    error,
    warn,
    info,

    last
};

constexpr std::string_view logLevelNames[] = {
    "[FATAL]: ",
    "[ERROR]: ",
    "[WARN]:  ",
    "[INFO]:  "
};

static_assert(static_cast<size_t>(LogLevel::last) == sizeof(logLevelNames)/sizeof(std::string_view), "Missing name for log level");

template<typename... Args>
void log(LogLevel level, std::format_string<Args...> fmt, Args &&...args)
{
    std::print("{}", logLevelNames[static_cast<size_t>(level)]);
    std::println(fmt, std::forward<Args>(args)...);
}

#define FATAL(...) log(LogLevel::fatal, __VA_ARGS__)
#define ERROR(...) log(LogLevel::error, __VA_ARGS__)

#ifdef DEBUG
    #define WARN(...) log(LogLevel::warn, __VA_ARGS__)
    #define INFO(...) log(LogLevel::info, __VA_ARGS__)
#else
    #define WARN(...)
    #define INFO(...)
#endif

