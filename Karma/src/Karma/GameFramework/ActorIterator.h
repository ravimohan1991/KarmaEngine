#pragma once

#include "krpch.h"

#include "KarmaUtilities.h"

namespace Karma
{
	class AActor;

	/**
	 * Actor iterator
	 * Note that when Playing In Editor, this will find actors only in CurrentWorld
	 */
	class FActorIterator : public TActorIteratorBase<FActorIterator>
	{
		friend class TActorIteratorBase<FActorIterator>;
		typedef TActorIteratorBase<FActorIterator> Super;

	public:
		/**
		 * Constructor
		 *
		 * @param InWorld	The world whose actors are to be iterated over.
		 * @param InFlags	Iteration flags indicating which types of levels and actors should be iterated
		 */
		explicit FActorIterator(const UWorld* InWorld, const EActorIteratorFlags InFlags = EActorIteratorFlags(OnlyActiveLevels | SkipPendingKill))
			: Super(InWorld, AActor::StaticClass(), InFlags)
		{
			++(*this);
		}

		/**
		 * Constructor
		 *
		 * @param InWorld	The world whose actors are to be iterated over.
		 * @param InClass	The type of actors to be iterated over.
		 * @param InFlags	Iteration flags indicating which types of levels and actors should be iterated
		 */
		explicit FActorIterator(const UWorld* InWorld, const TSubclassOf<AActor> InClass, const EActorIteratorFlags InFlags = EActorIteratorFlags(OnlyActiveLevels | SkipPendingKill))
			: Super(InWorld, InClass, InFlags)
		{
			++(*this);
		}
	};

	/**
	 * Template actor iterator.
	 */
	template <typename ActorType>
	class TActorIterator : public TActorIteratorBase<TActorIterator<ActorType>>
	{
		friend class TActorIteratorBase<TActorIterator>;
		typedef TActorIteratorBase<TActorIterator> Super;

	public:
		/**
		 * Constructor
		 *
		 * @param InWorld	The world whose actors are to be iterated over.
		 * @param InClass	The subclass of actors to be iterated over.
		 * @param InFlags	Iteration flags indicating which types of levels and actors should be iterated
		 */
		explicit TActorIterator(const UWorld* InWorld, TSubclassOf<AActor> InClass = ActorType::StaticClass(), EActorIteratorFlags InFlags = EActorIteratorFlags(EActorIteratorFlags::OnlyActiveLevels | EActorIteratorFlags::SkipPendingKill))
			: Super(InWorld, InClass, InFlags)
		{
			++(*this);
		}

		/**
		 * Constructor for creating an end iterator
		 */
		explicit TActorIterator(EActorIteratorType)
			: Super(EActorIteratorType::End)
		{
		}

		/**
		 * Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return	Current suitable actor
		 */
		FORCEINLINE ActorType* operator*() const
		{
			return CastChecked<ActorType>(**static_cast<const Super*>(this));
		}

		/**
		 * Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return	Current suitable actor
		 */
		FORCEINLINE ActorType* operator->() const
		{
			return **this;
		}
	};
}