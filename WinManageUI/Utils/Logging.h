#pragma once
#include <stdio.h>
#include <spdlog/spdlog.h>

#define DEBUG
                             
#ifdef DEBUG

#define OPEN_CONSOLE                                \
    AllocConsole();                                \
    FILE* stream;                                  \
    freopen_s(&stream, "CONOUT$", "w", stdout);    \
    freopen_s(&stream, "CONOUT$", "w", stderr);    \
    freopen_s(&stream, "CONIN$", "r", stdin);

#define LOG_INFO(...)    spdlog::info(__VA_ARGS__);

#define LOG_WARN(...)    spdlog::warn(__VA_ARGS__);

#define LOG_ERROR(...)   spdlog::error(__VA_ARGS__);

#define LOG_DEBUG(...)   spdlog::debug(__VA_ARGS__);

#define LOG_TRACE(...)   spdlog::trace(__VA_ARGS__);

#define LOG_CRITICAL(...) spdlog::critical(__VA_ARGS__);

#else

#define OPEN_CONSOLE
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_DEBUG(...)
#define LOG_TRACE(...)
#define LOG_CRITICAL(...)

#endif
