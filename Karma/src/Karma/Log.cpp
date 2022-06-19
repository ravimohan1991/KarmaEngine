#include "Log.h"

namespace Karma
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger = nullptr;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger = nullptr;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T][%l] %n: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("KARMA");
		s_CoreLogger->set_level(spdlog::level::trace);
		
		s_ClientLogger = spdlog::stdout_color_mt("APPLICATION");
		s_ClientLogger->set_level(spdlog::level::trace);

		KR_CORE_INFO("Initialized Log");
	}
}