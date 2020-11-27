#pragma once

#ifdef KR_WINDOWS_PLATFORM
	#ifdef KR_BUILD_DLL
		#define KARMA_API __declspec(dllexport)
	#else
		#define KARMA_API __declspec(dllimport)
	#endif
#else
	#error Supports Windows only!
#endif
