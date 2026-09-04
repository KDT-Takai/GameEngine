#include "pch/pch.h"
#include "DevLogSink.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Engine::Debugger::Logging
{
    void AttachFileLogSink()
    {
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            "../Debugger/Logs/latest.log", /*truncate=*/true);

        spdlog::default_logger()->sinks().push_back(fileSink);

        spdlog::flush_on(spdlog::level::trace);
    }
}
