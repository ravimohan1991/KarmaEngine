/**
 * @file Log.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the Log class. I want to imagine the Log pronounciation match with that of that https://youtu.be/dZr-53LAlOw?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&t=15
 * @version 1.0
 * @date December 4, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "spdlog/spdlog.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Karma
{
	/**
	 * @brief Karma's logging class with the logger like so
	 *
	 * Upon each log write the logger:
	 * 1. Checks if its log level is enough to log the message and if yes:
	 * 2. Call the underlying sinks to do the job.
	 * 3. Each sink use its own private copy of a formatter to format the message
	 * and send to its destination.
	 *
	 * The use of private formatter per sink provides the opportunity to cache some
	 * formatted data, and support for different format per sink.
	 */
	class KARMA_API Log
	{
	public:
		/**
		 * @brief Initializes the logging class
		 *
		 * The following steps are taken
		 * 1. Couple of spdlog::logger instances are created with name "Karma" and "Application" for Engine and Game contexts.
		 * 2. The logging patters is like so
		 * 	"%^[%T][%l] %n: %v%$" which has the example like so
		 * 	"[04:34:27][info] KARMA: Initialized Log"
		 *
		 * @since Karma 1.0.0
		 */
		static void Init();
		
		/**
		 * @brief Getter for the Core logger.
		 *
		 * @since Karma 1.0.0
		 */
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

		/**
		 * @brief Getter for the Client logger.
		 *
		 * @since Karma 1.0.0
		 */
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
/**
 * @brief A macro for logging error in Core part
 *
 * @since Karma 1.0.0
 */
#define KR_CORE_ERROR(...) ::Karma::Log::GetCoreLogger()->error(__VA_ARGS__)
/**
 * @brief A macro for logging warning in Core part
 *
 * @since Karma 1.0.0
 */
#define KR_CORE_WARN(...)  ::Karma::Log::GetCoreLogger()->warn(__VA_ARGS__)
/**
 * @brief A macro for logging information in the Core part
 *
 * @since Karma 1.0.0
 */
#define KR_CORE_INFO(...)  ::Karma::Log::GetCoreLogger()->info(__VA_ARGS__)
/**
 * @brief A macro for logging trace in the Core part
 *
 * @since Karma 1.0.0
 */
#define KR_CORE_TRACE(...) ::Karma::Log::GetCoreLogger()->trace(__VA_ARGS__)
/**
 * @brief A macro for logging fatal message in the Core part
 *
 * @since Karma 1.0.0
 */
#define KR_CORE_FATAL(...) ::Karma::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
/**
 * @brief A macro for logging error in the Client part
 *
 * @since Karma 1.0.0
 */
#define KR_ERROR(...) ::Karma::Log::GetClientLogger()->error(__VA_ARGS__)
/**
 * @brief A macro for logging warning in the Client part
 *
 * @since Karma 1.0.0
 */
#define KR_WARN(...)  ::Karma::Log::GetClientLogger()->warn(__VA_ARGS__)
/**
 * @brief A macro for logging information in the Client part
 *
 * @since Karma 1.0.0
 */
#define KR_INFO(...)  ::Karma::Log::GetClientLogger()->info(__VA_ARGS__)
/**
 * @brief A macro for logging trace in the Client part
 *
 * @since Karma 1.0.0
 */
#define KR_TRACE(...) ::Karma::Log::GetClientLogger()->trace(__VA_ARGS__)
/**
 * @brief A macro for logging fatal message in the Client part
 *
 * @since Karma 1.0.0
 */
#define KR_FATAL(...) ::Karma::Log::GetClientLogger()->fatal(__VA_ARGS__)
