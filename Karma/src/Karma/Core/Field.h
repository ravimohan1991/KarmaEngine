/**
 * @file Field.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class FField and relevant helpers.
 * @version 1.0
 * @date April 13, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "krpch.h"
#include "UObjectGlobals.h"

namespace Karma
{
	class UObject;
	class UClass;
	class FField;
	class FFieldClass;

	/**
	 * @brief Special container that can hold either UObject or FField.
	 *
	 * Exposes common interface of FFields and UObjects for easier transition from UProperties to FProperties.
	 * DO NOT ABUSE. IDEALLY THIS SHOULD ONLY BE FFIELD INTERNAL STRUCTURE FOR HOLDING A POINTER TO THE OWNER OF AN FFIELD.
	 *
	 * @remark Seems like data structure for reflection system
	 */
	class KARMA_API FFieldVariant
	{
		/**
		 * A union is a special class type that can hold only one of its non-static data members at a time.
		 * https://en.cppreference.com/w/cpp/language/union
		 */
		union FFieldObjectUnion
		{
			FField* Field;
			UObject* Object;
		} Container;

		bool bIsUObject;

	public:

		/**
		 * @brief Constructor
		 *
		 * @since Karma 1.0.0
		 */
		FFieldVariant()
			: bIsUObject(false)
		{
			Container.Field = nullptr;
		}

		/**
		 * @brief Overloaded constructor (FField)
		 *
		 * @param InField				The FField to initialize the union Container with
		 * @since Karma 1.0.0
		 */
		FFieldVariant(const FField* InField)
			: bIsUObject(false)
		{
			Container.Field = const_cast<FField*>(InField);
		}

		/**
		 * @brief Overloaded template constructor (UObject)
		 *
		 * @param InObject				The UObject to initialize the union Container with
		 * @since Karma 1.0.0
		 */
		template <
			typename T,
			decltype(ImplicitConv<const UObject*>(std::declval<T>()))* = nullptr
		>
		FFieldVariant(T&& InObject)
			: bIsUObject(true)
		{
			Container.Object = const_cast<UObject*>(ImplicitConv<const UObject*>(InObject));
		}

		/**
		 * @brief Getter for bIsUobject
		 *
		 * @return true if Container holds UObject only
		 * @since Karma 1.0.0
		 */
		inline bool IsUObject() const
		{
			return bIsUObject;
		}

		/**
		 * @brief Maybe sees if UObject is being held
		 *
		 * @todo Ponder over the routine
		 * @since Karma 1.0.0
		 */
		inline bool IsValid() const
		{
			return !!Container.Object;
		}

		/**
		 * No clue
		 * @warning May result in linking error because function not defined
		 *
		 * @todo Function definition is missing
		 */
		bool IsValidLowLevel() const;

		/**
		 * @brief A boolean conversion operator for the class
		 * https://en.cppreference.com/w/cpp/language/cast_operator
		 *
		 * @since Karma 1.0.0
		 */
		inline operator bool() const
		{
			return IsValid();
		}

		/**
		 * @todo Function definition is missing
		 */
		bool IsA(const UClass* InClass) const;

		/**
		 * @todo Function definition is missing
		 */
		bool IsA(const FFieldClass* InClass) const;

		template <typename T>
		bool IsA() const
		{
			static_assert(sizeof(T) > 0, "T must not be an incomplete type");
			return IsA(T::StaticClass());
		}

		/*
		template <typename T>
		typename TEnableIf<TIsDerivedFrom<T, UObject>::IsDerived, T*>::Type Get() const
		{
			static_assert(sizeof(T) > 0, "T must not be an incomplete type");
			if (IsA(T::StaticClass()))
			{
				return static_cast<T*>(Container.Object);
			}
			return nullptr;
		}

		template <typename T>
		typename TEnableIf<!TIsDerivedFrom<T, UObject>::IsDerived, T*>::Type Get() const
		{
			static_assert(sizeof(T) > 0, "T must not be an incomplete type");
			if (IsA(T::StaticClass()))
			{
				return static_cast<T*>(Container.Field);
			}
			return nullptr;
		}
		*/

		/**
		 * @brief Getter for UObject if possible
		 *
		 * @return UObject reference
		 * @since Karma 1.0.0
		 */
		UObject* ToUObject() const
		{
			if (bIsUObject)
			{
				return Container.Object;
			}
			else
			{
				return nullptr;
			}
		}

		/**
		 * @brief Getter for FField if possible
		 *
		 * @return FField reference
		 * @since Karma 1.0.0
		 */
		FField* ToField() const
		{
			if (!bIsUObject)
			{
				return Container.Field;
			}
			else
			{
				return nullptr;
			}
		}

		/** 
		 * FOR INTERNAL USE ONLY: Function that returns the owner as FField without checking if it's actually an FField
		 *
		 * @todo Ponder
		 * @since Karma 1.0.0
		 */
		FORCEINLINE FField* ToFieldUnsafe() const
		{
			return Container.Field;
		}

		/**
		 * FOR INTERNAL USE ONLY: Function that returns the owner as UObject without checking if it's actually a UObject
		 *
		 * @since Karma 1.0.0
		 */
		FORCEINLINE UObject* ToUObjectUnsafe() const
		{
			return Container.Object;
		}

		/**
		 * Seems like ToFieldUnsafe.
		 *
		 * @todo Ponder how this is different from previous similar function
		 */
		void* GetRawPointer() const
		{
			return Container.Field;
		}

		FFieldVariant GetOwnerVariant() const;
		UClass* GetOwnerClass() const;
		const std::string& GetFullName() const;
		const std::string& GetPathName() const;
		const std::string& GetName() const;
		const std::string& GetClassName() const;
		const std::string& GetFName() const;
		bool IsNative() const;
		//UPackage* GetOutermost() const;

		bool operator==(const FFieldVariant& Other) const
		{
			return Container.Field == Other.Container.Field;
		}
		
		bool operator!=(const FFieldVariant& Other) const
		{
			return Container.Field != Other.Container.Field;
		}
/*
#if WITH_EDITORONLY_DATA
		bool HasMetaData(const FName& Key) const;
#endif*/

		/** Support comparison functions that make this usable as a KeyValue for a TSet<> */
		/*friend uint32_t GetTypeHash(const FFieldVariant& InFieldVariant)
		{
			return GetTypeHash(InFieldVariant.GetRawPointer());
		}

		KARMA_API friend FArchive& operator << (FArchive& Ar, FFieldVariant& InOutField);*/
	};

	/**
	 * @brief Object representing a type of an FField struct.
	 * Mimics a subset of UObject reflection functions.
	 */
	class KARMA_API FFieldClass
	{
		/** Name of this field class */
		std::string m_Name;

		/** Unique Id of this field class (for casting) */
		uint64_t m_Id;
		
		/** Cast flags used for casting to other classes */
		uint64_t m_CastFlags;
		
		/** Class flags */
		EClassFlags ClassFlags;
		
		/** Super of this class */
		FFieldClass* SuperClass;
		
		/** Default instance of this class */
		FField* DefaultObject;
		
		/** Pointer to a function that can construct an instance of this class */
		FField* (*ConstructFn)(const FFieldVariant&, const std::string&, EObjectFlags);
		
		/** Counter for generating runtime unique names */
		//FThreadSafeCounter UnqiueNameIndexCounter;

		/** Creates a default object instance of this class */
		FField* ConstructDefaultObject();

	public:
		KR_NONCOPYABLE(FFieldClass);

		/**
		 * Gets the list of all field classes in existance
		 *
		 * @todo Write function definition when required
		 * @since Karma 1.0.0
		 */
		static KarmaVector<FFieldClass*>& GetAllFieldClasses();

		/**
		 * Gets a mapping of all field class names to the actual class objects
		 *
		 * @todo Write function definition when requreid
		 * @since Karma 1.0.0
		 */
		static std::unordered_map<std::string, FFieldClass*>& GetNameToFieldClassMap();

		/**
		 * Constructor
		 *
		 * @brief Initialize various data members of the class
		 */
		explicit FFieldClass(const char* InCPPName, uint64_t InId, uint64_t InCastFlags, FFieldClass* InSuperClass, FField* (*ConstructFnPtr)(const FFieldVariant&, const std::string&, EObjectFlags));

		/**
		 * Destructor
		 *
		 * @since Karma 1.0.0
		 */
		~FFieldClass();

		/**
		 * Getter for m_Name
		 *
		 * @since Karma 1.0.0
		 */
		inline const std::string& GetName() const
		{
			return m_Name;
		}
		
		/**
		 * Getter for m_Id
		 *
		 * @since Karma 1.0.0
		 */
		inline uint64_t GetId() const
		{
			return m_Id;
		}

		/**
		 * Getter for m_CastFlags
		 *
		 * @since Karma 1.0.0
		 */
		inline uint64_t GetCastFlags() const
		{
			return m_CastFlags;
		}
		inline bool HasAnyCastFlags(const uint64_t InCastFlags) const
		{
			return !!(m_CastFlags & InCastFlags);
		}
		inline bool HasAllCastFlags(const uint64_t InCastFlags) const
		{
			return (m_CastFlags & InCastFlags) == InCastFlags;
		}

		/**
		 * @brief See if the class is child of a class
		 *
		 * @todo Need to try with various cases for intended purpose
		 * @see FFieldClass::IsChildOf_Walk
		 *
		 * @since Karma 1.0.0
		 */
		inline bool IsChildOf(const FFieldClass* InClass) const
		{
			const uint64_t OtherClassId = InClass->GetId();
			return OtherClassId ? !!(m_CastFlags & OtherClassId) : IsChildOf_Walk(InClass);
		}

		const std::string& GetDescription() const;
		const std::string& GetDisplayNameText() const;

		FField* Construct(const FFieldVariant& InOwner, const std::string& InName, EObjectFlags InFlags = RF_NoFlags) const
		{
			return ConstructFn(InOwner, InName, InFlags);
		}

		FFieldClass* GetSuperClass() const
		{
			return SuperClass;
		}

		/**
		 * Getter for DefaultObject.
		 *
		 * @since Karma 1.0.0
		 */
		FField* GetDefaultObject()
		{
			if (!DefaultObject)
			{
				DefaultObject = ConstructDefaultObject();
				KR_CORE_ASSERT(DefaultObject, "Default object is null");
			}
			return DefaultObject;
		}

		/*
		bool HasAnyClassFlags(EClassFlags FlagsToCheck) const
		{
			return EnumHasAnyFlags(ClassFlags, FlagsToCheck) != 0;
		}

		int32_t GetNextUniqueNameIndex()
		{
			return UnqiueNameIndexCounter.Increment();
		}

		friend FArchive& operator << (FArchive& Ar, FFieldClass& InField)
		{
			check(false);
			return Ar;
		}
		KARMA_API friend FArchive& operator << (FArchive& Ar, FFieldClass*& InOutFieldClass);*/

	private:
		bool IsChildOf_Walk(const FFieldClass* InBaseClass) const
		{
			for (const FFieldClass* TempField = this; TempField; TempField = TempField->GetSuperClass())
			{
				if (TempField == InBaseClass)
				{
					return true;
				}
			}
			return false;
		}
	};

	/**
	 * @brief Base class of reflection data objects.
	 */
	class KARMA_API FField
	{
		/** Pointer to the class object representing the type of this FField */
		FFieldClass* ClassPrivate;

	public:
		KR_NONCOPYABLE(FField)

		typedef FField Super;
		typedef FField ThisClass;
		typedef FField BaseFieldClass;
		typedef FFieldClass FieldTypeClass;

		/** Owner of this field */
		FFieldVariant m_Owner;

		/** Next Field in the linked list */
		FField* m_Next;

		/** Name of this field */
		std::string m_NamePrivate;

		/** Object flags */
		EObjectFlags m_FlagsPrivate;

	public:
		/**
		 * @brief Rename the FField object
		 *
		 * @since Karma 1.0.0
		 */
		void Rename(const std::string& NewName);

		//static FFieldClass* StaticClass();
	};
}
