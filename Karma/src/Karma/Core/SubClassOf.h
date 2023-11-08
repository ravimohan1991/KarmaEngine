/**
 * @file SubClassOf.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class TSubclassOf and relevant helpers.
 * @version 1.0
 * @date April 13, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "krpch.h"

#include "Class.h"
#include "Field.h"

namespace Karma
{
	/** 
	 * @brief Chooses between two different classes based on a boolean.
	 */
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

	/**
	 * @brief Is type DerivedType inherited from BaseType?
	 *
	 * @remark Note that this is a hierarchy detecting mechanism similar to UObjectBase::IsA while
	 * being used for different purpose of direct Class type.  IsA is used for UClass hierarchy purpose used in game code (that we introduce by hand,
	 * more like uscript like hierarchy).
	 */
	template<typename DerivedType, typename BaseType>
	struct TIsDerivedFrom
	{
		// Different size types so we can compare their sizes later.
		/** When polymorphism fails, i.e there is no child-parent type relation for DerivedType and BaseType. */
		typedef char No[1];

		/** When polymorphism succeeds, i.e there is child-parent type relation for DerivedType and BaseType. */
		typedef char Yes[2];

		// Overloading Test() s.t. only calling it with something that is
		// a BaseType (or inherited from the BaseType) will return a Yes.

		/**
		 * @brief Handy function called when type-cast succeeds
		 *
		 * @param someBaseType				Pointer to (type-cast) BaseType
		 * @return Yes reference of specified size known from declaration and definition \ref Yes.
		 *
		 * @since Karma 1.0.0
		 */
		static Yes& Test(BaseType*);

		/**
		 * @brief Handy variant function called when type-cast succeeds
		 *
		 * @param someBaseType				Pointer to (type-cast) const BaseType
		 * @return Yes reference of specified size known from declaration and definition \ref Yes.
		 *
		 * @since Karma 1.0.0
		 */
		static Yes& Test(const BaseType*);

		/**
		 * @brief Handy function called when type-cast does not succeed
		 *
		 * @param ...	Whatever the DerivedType is
		 * @return No reference of specified size known from declaration and definition \ref No.
		 *
		 * @since Karma 1.0.0
		 */
		static No& Test(...);

		/**
		 * Makes a DerivedType ptr
		 *
		 * @return pointer to DerivedType
		 * @since Karma 1.0.0
		 */
		static DerivedType* DerivedTypePtr() { return nullptr; }

	public:
		/**
		 * Test the derived type pointer. If it inherits from BaseType, the Test( BaseType* )
		 * will be chosen. If it does not, Test( ... ) will be chosen.
		 */
		static constexpr bool Value = sizeof(Test(DerivedTypePtr())) == sizeof(Yes);

		static constexpr bool IsDerived = Value;
	};

	/**
	 * @brief Template to allow TClassType's to be passed around with type safety
	 */
	template<class TClass>
	class TSubclassOf
	{
	public:
		/**
		 * 
		 * @brief Since I don't think I understand the purpose of this typedef, I may attempt to parse the code.
		 * Seems like if TClass is derived from FField then TClassType is FFieldClass, else, UClass (for most scenarios). The pointer variable is of this type, i.e TClassType
		 */
		typedef typename TChooseClass<TIsDerivedFrom<TClass, FField>::IsDerived, FFieldClass, UClass>::Result TClassType;

		/**
		 * @brief Again, on parsing, I understand the meaning to be, if TClass is derived from FField then TBaseType is FField else UObject (for most scenarios).
		 * The return type of GetDefaultObject() is of this type, again UObject for majority of the cases.
		 *
		 */
		typedef typename TChooseClass<TIsDerivedFrom<TClass, FField>::IsDerived, FField, UObject>::Result TBaseType;

	private:
		template <class TClassA>
		friend class TSubclassOf;

	public:
		/** 
		 * Default Constructor, defaults to null
		 *
		 * @since Karma 1.0.0
		 */
		inline TSubclassOf() :
			Class(nullptr)
		{
		}

		/**
		 * Constructor that takes a UClass and does a runtime check to make sure this is a compatible class
		 *
		 * @param From Usually a UClass
		 * @since Karma 1.0.0
		 */
		inline TSubclassOf(TClassType* From) :
			Class(From)
		{
		}

		/** 
		 * @brief Constructor that takes a UClass and does a runtime check to make sure this is a compatible class
		 *
		 * Understanding by-parts
		 * 1. std::decay_t<U>: allowing the "pass by reference" of parameter (https://stackoverflow.com/a/25732651).
		 *  Basically an equivalence is established between, for instance, int(&)[2] and int* (reference to an array, even whole array, and pointer to appropriate data type)
		 *
		 * 2. std::declval<U>: Converts any type U to a reference type, allowing the use of class (struct) functions of U without actually initializing an
		 * object (simillar to that of static functions, in the sense no object is needed). For instance:
		 *
		 * 	@code{.cpp}
		 * 	struct NonDefault
		 * 	{
		 * 		NonDefault() = delete;
		 * 		int foo() const { return 1; }
		 * 	};
		 * 	decltype(Default().foo()) n1 = 1;
		 * 	@endcode
		 *
		 * 	Note the usage of foo() function without creating the object
		 *
		 * 3. ImplicitConv<TClassType*>: An Unreal Engine function, uses implicit conversion to create an instance of a specific type. Not functional here yet
		 *
		 * 4. decltype: The decltype keyword inspects the declared type of an entity or the type of an expression. ‘auto’ lets you declare a variable with a
		 * particular type whereas decltype lets you extract the type from the variable so decltype is sort of an operator that evaluates the type of passed expression.
		 *
		 * 5. std::enable_if_t<U>: Consider the following statement
		 *
		 * 	@code{.cpp}
		 * 	template< bool B, class T = void >
		 * 	struct enable_if;
		 * 	@endcode
		 *
		 *	then, If B is true, std::enable_if has a public member typedef type, equal to T; otherwise,
		 *	there is no member typedef.
		 *
		 * 	@code{.cpp}
		 * 	template< bool B, class T = void >
		 * 	using enable_if_t = typename enable_if<B,T>::type;
		 * 	@endcode
		 *
		 * @param From	A (mostly) UClass pointer with runtime check (which I don't practically understand)
		 *
		 * @todo Definition for ImplicitConv needs to be provided
		 * @since Karma 1.0.0
		 */
		template <
			typename U,
			std::enable_if_t<!TIsTSubclassOf<std::decay_t<U>>::Value, decltype(ImplicitConv<TClassType*>(std::declval<U>()))
			>* = nullptr
		>
		inline TSubclassOf(U&& From)
			: Class(From)
		{
		}

		/** 
		 * Copy Constructor, will only compile if types are compatible
		 *
		 * @param From 	A derived TClassA
		 * @since Karma 1.0.0
		 */
		template <class TClassA, class = decltype(ImplicitConv<TClass*>((TClassA*)nullptr))>
		inline TSubclassOf(const TSubclassOf<TClassA>& From) :
			Class(*From)
		{
		}

		/** 
		 * Assignment operator, will only compile if types are compatible
		 *
		 * @param From 	The object to copy from
		 * @since Karma 1.0.0
		 */
		template <class TClassA, class = decltype(ImplicitConv<TClass*>((TClassA*)nullptr))>
		inline TSubclassOf& operator=(const TSubclassOf<TClassA>& From)
		{
			Class = *From;
			return *this;
		}

		/** 
		 * Assignment operator from UClass, the type is checked on get not on set
		 *
		 * @param From 	The object to copy from
		 * @since Karma 1.0.0
		 */
		inline TSubclassOf& operator=(TClassType* From)
		{
			Class = From;
			return *this;
		}

		/** 
		 * Assignment operator from UClass, the type is checked on get not on set
		 *
		 * @param From 	Object to copy from
		 * @since Karma 1.0.0
		 */
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

		/** 
		 * Dereference back into a UClass, does runtime type checking
		 *
		 * @note Not seen practical example
		 * @since Karma 1.0.0
		 */
		inline TClassType* operator*() const
		{
			if (!Class || !Class->IsChildOf(TClass::StaticClass()))
			{
				return nullptr;
			}
			return Class;
		}

		/** 
		 * Dereference back into a UClass
		 *
		 * @since Karma 1.0.0
		 */
		inline TClassType* Get() const
		{
			return **this;
		}

		/** 
		 * Dereference back into a UClass
		 *
		 * @since Karma 1.0.0
		 */
		inline TClassType* operator->() const
		{
			return **this;
		}

		/** 
		 * Implicit conversion to UClass
		 *
		 * @since Karma 1.0.0
		 */
		inline operator TClassType* () const
		{
			return **this;
		}

		/**
		 * Get the CDO if we are referencing a valid class
		 *
		 * @return the CDO, or null if class is null
		 * @since Karma 1.0.0
		 */
		inline TClass* GetDefaultObject() const
		{
			TBaseType* Result = nullptr;
			if (Class)
			{
				Result = Class->GetDefaultObject();
				KR_CORE_ASSERT(Result && Result->IsA(TClass::StaticClass()), "");
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
