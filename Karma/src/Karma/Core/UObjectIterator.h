#pragma once

#include "krpch.h"
#include "UObjectGlobals.h"


namespace Karma
{

	/**
	 * Class for iterating through all objects, including class default objects, unreachable objects...all UObjects
	 */
	class FRawObjectIterator : public FUObjectArray::TIterator
	{
	public:
		/**
		 * Constructor
		 * @param	bOnlyGCedObjects	if true, skip all of the permanent objects
		 */
		FRawObjectIterator(bool bOnlyGCedObjects = false) :
			FUObjectArray::TIterator(GUObjectStore, bOnlyGCedObjects)
		{
		}
		/**
		* Iterator dereference
		* @return	the object pointer pointed at by the iterator
		*/
		FORCEINLINE FUObjectItem* operator*() const
		{
			// casting UObjectBase to UObject for clients
			return GetObject();
		}

		/**
		 * Iterator dereference
		 * @return	the object pointer pointed at by the iterator
		 */
		FORCEINLINE FUObjectItem* operator->() const
		{
			return GetObject();
		}
};

	/*
	inline EInternalObjectFlags GetObjectIteratorDefaultInternalExclusionFlags(EInternalObjectFlags InternalExclusionFlags)
	{
		//InternalExclusionFlags = UObjectBaseUtility::FixGarbageOrPendingKillInternalObjectFlags(InternalExclusionFlags);

		InternalExclusionFlags =  EInternalObjectFlags(int32_t(InternalExclusionFlags) | (int32_t(EInternalObjectFlags::Unreachable) | int32_t(EInternalObjectFlags::PendingConstruction)));

		if (!IsInAsyncLoadingThread())
		{
			// We don't want to return any objects that are currently being background loaded unless we're using the object iterator during async loading.
			InternalExclusionFlags |= EInternalObjectFlags::AsyncLoading;
		}

		return InternalExclusionFlags;
	}*/

	/**
	 * Class for iterating through all UObjects.  Does not include any
	 * class default objects.
	 * Note that when Playing In Editor, this will find objects in the
	 * editor as well as the PIE world, in an indeterminate order.
	 *
	 * I think we are branching off from UE, untill I figure the philosophical difference
	 * between TObjectIterator and TActorIterator
	 */
	/*template<class T> class TObjectIterator
	{
	public:
		enum EEndTagType
		{
			EndTag
		};


		explicit TObjectIterator(EObjectFlags AdditionalExclusionFlags = RF_ClassDefaultObject, bool bIncludeDerivedClasses = true, EInternalObjectFlags InInternalExclusionFlags = EInternalObjectFlags::None)
			: m_Index(-1)
		{
			GetObjectsOfClass(T::StaticClass(), m_ObjectArray, bIncludeDerivedClasses, AdditionalExclusionFlags, GetObjectIteratorDefaultInternalExclusionFlags(InInternalExclusionFlags));
			Advance();
		}


		TObjectIterator(EEndTagType, TObjectIterator& Begin)
			: m_Index(Begin.m_ObjectArray.Num())
		{
		}


		FORCEINLINE void operator++()
		{
			Advance();
		}


		FORCEINLINE explicit operator bool() const
		{
			return m_ObjectArray.IsValidIndex(m_Index);
		}


		FORCEINLINE bool operator !() const
		{
			return !(bool)*this;
		}


		FUNCTION_NON_NULL_RETURN_START
			FORCEINLINE T* operator* () const
		FUNCTION_NON_NULL_RETURN_END
		{
			return (T*)GetObject();
		}

		FUNCTION_NON_NULL_RETURN_START
			FORCEINLINE T* operator-> () const
		FUNCTION_NON_NULL_RETURN_END
		{
			return (T*)GetObject();
		}

		FORCEINLINE bool operator==(const TObjectIterator& Rhs) const { return m_Index == Rhs.Index; }
		FORCEINLINE bool operator!=(const TObjectIterator& Rhs) const { return m_Index != Rhs.Index; }

	protected:

		FORCEINLINE UObject* GetObject() const
		{
			return m_ObjectArray.GetElements()[m_Index];
		}


		FORCEINLINE bool Advance()
		{
			//@todo UE check this for LHS on Index on consoles
			while(++m_Index < m_ObjectArray.Num())
			{
				if (GetObject())
				{
					return true;
				}
			}
			return false;
		}

	protected:

		KarmaVector<UObject*> m_ObjectArray;

		int32_t m_Index;
	};
*/
}
