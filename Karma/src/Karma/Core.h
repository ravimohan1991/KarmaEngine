#pragma once

#ifdef KR_DYNAMIC_LINK
#ifdef KR_WINDOWS_PLATFORM
	#ifdef KR_BUILD_DLL
		#define KARMA_API __declspec(dllexport)
	#else
		#define KARMA_API __declspec(dllimport)
	#endif
#elif defined KR_LINUX_PLATFORM
    #ifdef KR_BUILD_SO
        #define KARMA_API __attribute__((visibility("default")))
    #else
        #define KARMA_API
    #endif
#elif defined KR_MAC_PLATFORM
    #ifdef KR_BUILD_SO
        #define KARMA_API __attribute__((visibility("default")))
    #else
        #define KARMA_API
    #endif
#else
#error Unsupported Platform detected!
#endif
#else
#define KARMA_API
#endif

#define BIT(x) (1 << x)

// Assertions
#ifdef KR_ENABLE_ASSERTS
	#if _MSC_VER
		#include <intrin.h>
		#define debugBreak() __debugbreak()
	#else
		#include <signal.h>
		#define debugBreak() raise(SIGTRAP)
	#endif

	#define KR_ASSERT(expr, ...) \
			if(expr){} \
			else \
			{\
				KR_ERROR("Assertion Failed: {0}. Refer file: {1}, line: {2}", __VA_ARGS__, __FILE__, __LINE__); \
				debugBreak(); \
			}
	#define KR_CORE_ASSERT(expr, ...) \
			if(expr){} \
			else \
			{\
				KR_CORE_ERROR("Assertion Failed: {0}. Refer file: {1}, line: {2}", __VA_ARGS__, __FILE__, __LINE__); \
				debugBreak(); \
			}
#else
	#define KR_ASSERT(expr, ...)
	#define KR_CORE_ASSERT(expr, ...)
#endif

#define KR_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)