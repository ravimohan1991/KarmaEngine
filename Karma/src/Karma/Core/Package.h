#pragma once

#include "krpch.h"
#include "Object.h"

namespace Karma
{
	class KARMA_API UPackage : public UObject
	{
		DECLARE_KARMA_CLASS(UPackage, UObject)

	public:
		/**
		 * A constructor
		 */
		UPackage();

		/**
		 * Set the specified flags to true. Does not affect any other flags.
		 *
		 * @param	NewFlags		Package flags to enable
		 */
		FORCEINLINE void SetPackageFlags(uint32_t NewFlags)
		{
			SetPackageFlagsTo(m_PackageFlagsPrivate | NewFlags);
		}

		FORCEINLINE void SetPackageFlagsTo(uint32_t NewFlags)
		{
			m_PackageFlagsPrivate = NewFlags;
		}

		/**
		 * Called to indicate that this package contains a ULevel or UWorld object.
		 */
		void ThisContainsMap()
		{
			SetPackageFlags(PKG_ContainsMap);
		}

	private:
		/** Package Flags */
		uint32_t	m_PackageFlagsPrivate;
	};
}
