#pragma once
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
//#include <spdlog/spdlog.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Utility/Singleton/Singleton.hpp"

#ifdef _DEBUG
// Debugger (Debug-only; not linked/built in Release): adds file logging.
#include "Logging/DevLogSink.hpp"
#endif

namespace Engine::Utility
{
    class Logger : public Singleton<Logger>
    {
        DECLARE_SINGLETON(Logger);

    private:
        Logger()
        {
            // Console sink: colored output for interactive runs.
            // Always present, in both Debug and Release.
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            auto logger = std::make_shared<spdlog::logger>(
                "default", spdlog::sinks_init_list{ consoleSink });

            spdlog::set_default_logger(logger);
            spdlog::set_pattern("[%H:%M:%S][%^%l%$][%s:%#] %v");
            spdlog::set_level(spdlog::level::trace);

#ifdef _DEBUG
            // File logging (Logs/latest.log) is a Debugger feature: it lets
            // the latest run's log be read straight from disk (e.g. by
            // Claude Code) without needing console output pasted in. Not
            // present in Release builds.
            Engine::Debugger::Logging::AttachFileLogSink();
#endif
        }

        ~Logger() = default;
    };
}

#define LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)
#define HR_LOG(hr) static_cast<uint32_t>(hr)
#define LOG_HRESULT(msg, hr) LOG_ERROR("{}: 0x{:08X}", msg, static_cast<uint32_t>(hr))