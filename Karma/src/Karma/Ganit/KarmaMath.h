/**
 * @file KarmaMath.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains some generally used Math functions.
 * @version 1.0
 * @date March 8, 2023
 *
 * @copyright Karma Engine: copyright(c) People of India
 */

#pragma once

#include "krpch.h"

#include "glm/common.hpp"

namespace Karma
{
	/**
	 * @brief A general purpose structure for Math functions and utilities
	 */
	struct KARMA_API FMath
	{
		/** 
		 * @brief Returns higher value in a generic way
		 *
		 * @param A				One of the two variables for comparison
		 * @param B				One of the two variables for comparison
		 *
		 * @return Maximum from A and B using glm library
		 * @since Karma 1.0.0
		 */
		template< class T >
		static  T Max(const T A, const T B)
		{
			return glm::max(A, B);
		}

		/** 
		 * @brief Returns lower value in a generic way
		 *
		 * @param A				One of the two variables for comparison
		 * @param B				One of the two variables for comparison
		 *
		 * @return Minimum from A and B
		 * @since Karma 1.0.0
		 */
		template< class T >
		static  T Min(const T A, const T B)
		{
			return glm::min(A, B);
		}

		/**
		 * @brief No clue what this does
		 *
		 * @todo Not yet functional because no clear
		 * @see For usage, see FGenericPlatformMemory::MemswapGreaterThan8
		 *
		 * @since Karma 1.0.0
		 */
		static uint32_t CountTrailingZeros(uint32_t Value)
		{
			// return 32 if value was 0
			unsigned long BitIndex = 0;	// 0-based, where the LSB is 0 and MSB is 31
			return Value;//_BitScanForward(&BitIndex, Value) ? BitIndex : 32;
		}
	};
}
