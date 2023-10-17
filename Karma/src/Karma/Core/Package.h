/**
 * @file Package.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UPackage.
 * @version 1.0
 * @date April 13, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "krpch.h"
#include "Object.h"

namespace Karma
{
	/**
	 * @brief A generic outer for UObjects, for instance UWorld
	 * @note The definition (or brief) is subjected to change on further Engine development
	 */
	class KARMA_API UPackage : public UObject
	{
		DECLARE_KARMA_CLASS(UPackage, UObject)

	public:
		/**
		 * A constructor
		 *
		 * @since Karma 1.0.0
		 */
		UPackage();

		/**
		 * Set the specified flags to true. Does not affect any other flags.
		 *
		 * @param NewFlags		Package flags to enable
		 * @since Karma 1.0.0
		 */
		FORCEINLINE void SetPackageFlags(uint32_t NewFlags)
		{
			SetPackageFlagsTo(m_PackageFlagsPrivate | NewFlags);
		}

		/**
		 * @brief Completely rewrites the older flags
		 *
		 * @since Karma 1.0.0
		 */
		FORCEINLINE void SetPackageFlagsTo(uint32_t NewFlags)
		{
			m_PackageFlagsPrivate = NewFlags;
		}

		/**
		 * @brief Called to indicate that this package contains a ULevel or UWorld object.
		 *
		 * @see UWorld::CreateWorld
		 * @since Karma 1.0.0
		 */
		void ThisContainsMap()
		{
			SetPackageFlags(PKG_ContainsMap);
		}

		/**
		 * Used to safely check whether the passed in flag is set.
		 *
		 * @param	FlagsToCheck		Package flags to check for
		 *
		 * @return	true if the passed in flag is set, false otherwise
		 *			(including no flag passed in, unless the FlagsToCheck is CLASS_AllFlags)
		 *
		 * @since Karma 1.0.0
		 */
		FORCEINLINE bool HasAnyPackageFlags(uint32_t FlagsToCheck) const
		{
			return (m_PackageFlagsPrivate & FlagsToCheck) != 0;
		}

	private:
		/** Package Flags */
		uint32_t	m_PackageFlagsPrivate;
	};
}
