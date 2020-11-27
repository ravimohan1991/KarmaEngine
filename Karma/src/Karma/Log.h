#pragma once

#include <memory>

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Karma
{
	class KARMA_API Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define KR_CORE_ERROR(...) ::Karma::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KR_CORE_WARN(...)  ::Karma::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KR_CORE_INFO(...)  ::Karma::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KR_CORE_TRACE(...) ::Karma::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KR_CORE_FATAL(...) ::Karma::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define KR_ERROR(...) ::Karma::Log::GetClientLogger()->error(__VA_ARGS__)
#define KR_WARN(...)  ::Karma::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KR_INFO(...)  ::Karma::Log::GetClientLogger()->info(__VA_ARGS__)
#define KR_TRACE(...) ::Karma::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KR_FATAL(...) ::Karma::Log::GetClientLogger()->fatal(__VA_ARGS__)