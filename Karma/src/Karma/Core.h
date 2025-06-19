/**
 * @file Core.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the macros for Karma's classes' general purpose use, including assertions and storage-class information (for multiplatforms).
 * @version 1.0
 * @date November 27, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

/**
 * @brief Defining Karma's API macro for storage class information
 *
 * @note This is platform based for dynamic linking (dll or shared object, or dylib)
 * @since Karma 1.0.0
 */
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

/**
 * @brief Macro for bit-shift operation (left)
 *
 * Basically shifting 0000001 to x power of 2 (2^x).
 * BIT(1) is 0000010
 *
 * @since Karma 1.0.0
 */
#define BIT(x) (1 << x)

// Assertions
/**
 * @brief Karma's Assertion
 *
 * @note The KR_ENABLE_ASSERTS must be defined
 * @since Karma 1.0.0
 */
#ifdef KR_ENABLE_ASSERTS
#include "Karma/Log.h"
	/**
	 * @brief For stopping the code execution
	 *
	 * @since Karma 1.0.0
	 */
	#if _MSC_VER
		#include <intrin.h>
		#define debugBreak() __debugbreak()
	#else
		#include <signal.h>
		#define debugBreak() raise(SIGTRAP)
	#endif

	/**
	 * @brief Karma's Assertion for application
	 *
	 * @note The KR_ENABLE_ASSERTS must be defined
	 * @since Karma 1.0.0
	 */
	#define KR_ASSERT(expr, ...) \
			if(expr){} \
			else \
			{\
				KR_ERROR("Assertion Failed: {0}. Refer file: {1}, line: {2}", __VA_ARGS__, __FILE__, __LINE__); \
				debugBreak(); \
			}
	/**
	 * @brief Karma's Assertion for Engine
	 *
	 * @note The KR_ENABLE_ASSERTS must be defined
	 * @since Karma 1.0.0
	 */
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

/**
 * @brief Macro for std::bind routine. See https://en.cppreference.com/w/cpp/utility/functional/bind
 *
 * @since Karma 1.0.0
 */
#define KR_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

/**
 * @brief Makes a type non-copyable and non-movable by deleting copy/move constructors and assignment/move operators.
 * The macro should be placed in the public section of the type for better compiler diagnostic messages.
 * Example usage:
 * @code{.cpp}
 *	class FMyClassName
 *	{
 *	public:
 *		UE_NONCOPYABLE(FMyClassName)
 *		FMyClassName() = default;
 *	};
 * @endcode
 */
#define KR_NONCOPYABLE(TypeName) \
	TypeName(TypeName&&) = delete; \
	TypeName(const TypeName&) = delete; \
	TypeName& operator=(const TypeName&) = delete; \
	TypeName& operator=(TypeName&&) = delete;


// Need to write Platform.h and WindowsPlatform.h specialization for these Unreal type of defines
// only for MSW
#define FUNCTION_NON_NULL_RETURN_START //_Ret_notnull_				/* Indicate that the function never returns nullptr. */

/* Wrap a function signature in these to indicate that the function never returns nullptr */
#ifndef FUNCTION_NON_NULL_RETURN_START
#define FUNCTION_NON_NULL_RETURN_START
#endif
#ifndef FUNCTION_NON_NULL_RETURN_END
#define FUNCTION_NON_NULL_RETURN_END
#endif

/*
 * For mac or appleclang inline
 * for MSW __forceinline
 */

/**
 * @brief Typical inlining macro for clarity
 *
 * @since Karma 1.0.0
 */
#ifndef FORCEINLINE
#define FORCEINLINE inline
#endif
