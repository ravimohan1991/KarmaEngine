#pragma once

#include "krpch.h"

#include "glm/common.hpp"

namespace Karma
{
#define FORCEINLINE __forceinline									/* Force code to be inline */

	struct FMath// KARMA_API?
	{
		/** Returns higher value in a generic way */
		template< class T >
		static  T Max(const T A, const T B)
		{
			return glm::max(A, B);
		}

		/** Returns lower value in a generic way */
		template< class T >
		static  T Min(const T A, const T B)
		{
			return glm::min(A, B);
		}

		static FORCEINLINE uint32_t CountTrailingZeros(uint32_t Value)
		{
			// return 32 if value was 0
			unsigned long BitIndex;	// 0-based, where the LSB is 0 and MSB is 31
			return _BitScanForward(&BitIndex, Value) ? BitIndex : 32;
		}
	};
}