/**
 * @file ActorIterator.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UActorComponent.
 * @version 1.0
 * @date February 27, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "KarmaUtilities.h"

namespace Karma
{
	class AActor;

	/**
	 * @brief Actor iterator
	 *
	 * @note When Playing In Editor, this will find actors only in CurrentWorld
	 * @note Seems like not in use
	 */
	class FActorIterator : public TActorIteratorBase<FActorIterator>
	{
		/**
		 * @brief Maybe for use in future
		 *
		 * @since Karma 1.0.0
		 */
		friend class TActorIteratorBase<FActorIterator>;

		/**
		 * @brief Naming, for familiar recollectable coding style
		 *
		 * @since Karma 1.0.0
		 */
		typedef TActorIteratorBase<FActorIterator> Super;

	public:
		/**
		 * @brief A Constructor
		 *
		 * @param InWorld	The world whose actors are to be iterated over.
		 * @param InFlags	Iteration flags indicating which types of levels and actors should be iterated
		 *
		 * @since Karma 1.0.0
		 */
		explicit FActorIterator(const UWorld* InWorld, const EActorIteratorFlags InFlags = EActorIteratorFlags(OnlyActiveLevels | SkipPendingKill))
			: Super(InWorld, AActor::StaticClass(), InFlags)
		{
			++(*this);
		}

		/**
		 * @brief A Constructor
		 *
		 * @param InWorld	The world whose actors are to be iterated over.
		 * @param InClass	The type of actors to be iterated over.
		 *
		 * @param InFlags	Iteration flags indicating which types of levels and actors should be iterated
		 *
		 * @since Karma 1.0.0
		 */
		explicit FActorIterator(const UWorld* InWorld, const TSubclassOf<AActor> InClass, const EActorIteratorFlags InFlags = EActorIteratorFlags(OnlyActiveLevels | SkipPendingKill))
			: Super(InWorld, InClass, InFlags)
		{
			++(*this);
		}
	};

	/**
	 * @brief Template actor iterator.
	 *
	 * @note The use is like so
	 * @code{.cpp}
	 * 	for (TActorIterator<AActor> ActorItr(testWorld); ActorItr; ++ActorItr)
	 * 	{
	 * 		KR_INFO("Iterating over actor: {0}", (*ActorItr)->GetName());
	 * 	}
	 * @endcode
	 */
	template <typename ActorType>
	class TActorIterator : public TActorIteratorBase<TActorIterator<ActorType>>
	{
		friend class TActorIteratorBase<TActorIterator>;
		typedef TActorIteratorBase<TActorIterator> Super;

	public:
		/**
		 * @brief A Constructor
		 *
		 * @param InWorld	The world whose actors are to be iterated over.
		 * @param InClass	The subclass of actors to be iterated over.
		 *
		 * @param InFlags	Iteration flags indicating which types of levels and actors should be iterated
		 *
		 * @since Karma 1.0.0
		 */
		explicit TActorIterator(const UWorld* InWorld, TSubclassOf<AActor> InClass = ActorType::StaticClass(), EActorIteratorFlags InFlags = EActorIteratorFlags(EActorIteratorFlags::OnlyActiveLevels | EActorIteratorFlags::SkipPendingKill))
			: Super(InWorld, InClass, InFlags)
		{
			++(*this);
		}

		/**
		 * @brief Constructor for creating an end iterator
		 *
		 * @since Karma 1.0.0
		 */
		explicit TActorIterator(EActorIteratorType)
			: Super(EActorIteratorType::End)
		{
		}

		/**
		 * @brief Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return	Current suitable actor
		 * @since Karma 1.0.0
		 */
		FORCEINLINE ActorType* operator*() const
		{
			return CastChecked<ActorType>(**static_cast<const Super*>(this));
		}

		/**
		 * @brief Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return	Current suitable actor
		 * @since Karma 1.0.0
		 */
		FORCEINLINE ActorType* operator->() const
		{
			return **this;
		}
	};
}
