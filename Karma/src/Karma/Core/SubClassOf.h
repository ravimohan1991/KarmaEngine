#pragma once

#include "krpch.h"

#include "Class.h"
#include "Field.h"

namespace Karma
{
	/** Chooses between two different classes based on a boolean. */
	template<bool Predicate, typename TrueClass, typename FalseClass>
	class TChooseClass;

	template<typename TrueClass, typename FalseClass>
	class TChooseClass<true, TrueClass, FalseClass>
	{
	public:
		typedef TrueClass Result;
	};

	template<typename TrueClass, typename FalseClass>
	class TChooseClass<false, TrueClass, FalseClass>
	{
	public:
		typedef FalseClass Result;
	};

	template <typename T>
	struct TIsTSubclassOf
	{
		enum { Value = false };
	};

	/** Is type DerivedType inherited from BaseType. */
	template<typename DerivedType, typename BaseType>
	struct TIsDerivedFrom
	{
		// Different size types so we can compare their sizes later.
		typedef char No[1];
		typedef char Yes[2];

		// Overloading Test() s.t. only calling it with something that is
		// a BaseType (or inherited from the BaseType) will return a Yes.
		static Yes& Test(BaseType*);
		static Yes& Test(const BaseType*);
		static No& Test(...);

		// Makes a DerivedType ptr.
		static DerivedType* DerivedTypePtr() { return nullptr; }

	public:
		// Test the derived type pointer. If it inherits from BaseType, the Test( BaseType* ) 
		// will be chosen. If it does not, Test( ... ) will be chosen.
		static constexpr bool Value = sizeof(Test(DerivedTypePtr())) == sizeof(Yes);

		static constexpr bool IsDerived = Value;
	};

	/**
	 * Template to allow TClassType's to be passed around with type safety
	 */
	template<class TClass>
	class TSubclassOf
	{
	public:
		typedef typename TChooseClass<TIsDerivedFrom<TClass, FField>::IsDerived, FFieldClass, UClass>::Result TClassType;
		typedef typename TChooseClass<TIsDerivedFrom<TClass, FField>::IsDerived, FField, UObject>::Result TBaseType;

	private:
		template <class TClassA>
		friend class TSubclassOf;

	public:
		/** Default Constructor, defaults to null */
		inline TSubclassOf() :
			Class(nullptr)
		{
		}

		/** Constructor that takes a UClass and does a runtime check to make sure this is a compatible class */
		inline TSubclassOf(TClassType* From) :
			Class(From)
		{
		}

		/** Constructor that takes a UClass and does a runtime check to make sure this is a compatible class */
		template <
			typename U,
			std::enable_if_t<
			!TIsTSubclassOf<std::decay_t<U>>::Value,
			decltype(ImplicitConv<TClassType*>(std::declval<U>()))
			>* = nullptr
		>
		inline TSubclassOf(U&& From)
			: Class(From)
		{
		}

		/** Copy Constructor, will only compile if types are compatible */
		template <class TClassA, class = decltype(ImplicitConv<TClass*>((TClassA*)nullptr))>
		inline TSubclassOf(const TSubclassOf<TClassA>& From) :
			Class(*From)
		{
		}

		/** Assignment operator, will only compile if types are compatible */
		template <class TClassA, class = decltype(ImplicitConv<TClass*>((TClassA*)nullptr))>
		inline TSubclassOf& operator=(const TSubclassOf<TClassA>& From)
		{
			Class = *From;
			return *this;
		}

		/** Assignment operator from UClass, the type is checked on get not on set */
		inline TSubclassOf& operator=(TClassType* From)
		{
			Class = From;
			return *this;
		}

		/** Assignment operator from UClass, the type is checked on get not on set */
		template <
			typename U,
			std::enable_if_t<
			!TIsTSubclassOf<std::decay_t<U>>::Value,
			decltype(ImplicitConv<TClassType*>(std::declval<U>()))
			>* = nullptr
		>
		inline TSubclassOf& operator=(U&& From)
		{
			Class = From;
			return *this;
		}

		/** Dereference back into a UClass, does runtime type checking */
		inline TClassType* operator*() const
		{
			if (!Class || !Class->IsChildOf(TClass::StaticClass()))
			{
				return nullptr;
			}
			return Class;
		}

		/** Dereference back into a UClass */
		inline TClassType* Get() const
		{
			return **this;
		}

		/** Dereference back into a UClass */
		inline TClassType* operator->() const
		{
			return **this;
		}

		/** Implicit conversion to UClass */
		inline operator TClassType* () const
		{
			return **this;
		}

		/**
		 * Get the CDO if we are referencing a valid class
		 *
		 * @return the CDO, or null if class is null
		 */
		inline TClass* GetDefaultObject() const
		{
			TBaseType* Result = nullptr;
			if (Class)
			{
				Result = Class->GetDefaultObject();
				check(Result && Result->IsA(TClass::StaticClass()));
			}
			return (TClass*)Result;
		}
		/*
		friend FArchive& operator<<(FArchive& Ar, TSubclassOf& SubclassOf)
		{
			Ar << SubclassOf.Class;
			return Ar;
		}

		friend uint32 GetTypeHash(const TSubclassOf& SubclassOf)
		{
			return GetTypeHash(SubclassOf.Class);
		}*/

	private:
		TClassType* Class;
	};

	template <typename T>
	struct TIsTSubclassOf<TSubclassOf<T>>
	{
		enum { Value = true };
	};
}