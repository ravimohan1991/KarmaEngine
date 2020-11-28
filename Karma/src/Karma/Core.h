#pragma once

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
#else
#error Unsupported Platform detected!
#endif
