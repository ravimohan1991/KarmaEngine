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
	 * Special container that can hold either UObject or FField.
	 * Exposes common interface of FFields and UObjects for easier transition from UProperties to FProperties.
	 * DO NOT ABUSE. IDEALLY THIS SHOULD ONLY BE FFIELD INTERNAL STRUCTURE FOR HOLDING A POINTER TO THE OWNER OF AN FFIELD.
	 */
	class KARMA_API FFieldVariant
	{
		union FFieldObjectUnion
		{
			FField* Field;
			UObject* Object;
		} Container;

		bool bIsUObject;

	public:

		FFieldVariant()
			: bIsUObject(false)
		{
			Container.Field = nullptr;
		}

		FFieldVariant(const FField* InField)
			: bIsUObject(false)
		{
			Container.Field = const_cast<FField*>(InField);
		}

		template <
			typename T,
			decltype(ImplicitConv<const UObject*>(std::declval<T>()))* = nullptr
		>
		FFieldVariant(T&& InObject)
			: bIsUObject(true)
		{
			Container.Object = const_cast<UObject*>(ImplicitConv<const UObject*>(InObject));
		}

		inline bool IsUObject() const
		{
			return bIsUObject;
		}
		inline bool IsValid() const
		{
			return !!Container.Object;
		}
		bool IsValidLowLevel() const;
		inline operator bool() const
		{
			return IsValid();
		}
		bool IsA(const UClass* InClass) const;
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
		/** FOR INTERNAL USE ONLY: Function that returns the owner as FField without checking if it's actually an FField */
		FORCEINLINE FField* ToFieldUnsafe() const
		{
			return Container.Field;
		}
		/** FOR INTERNAL USE ONLY: Function that returns the owner as UObject without checking if it's actually a UObject */
		FORCEINLINE UObject* ToUObjectUnsafe() const
		{
			return Container.Object;
		}

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
	 * Object representing a type of an FField struct.
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

		/** Gets the list of all field classes in existance */
		static KarmaVector<FFieldClass*>& GetAllFieldClasses();

		/** Gets a mapping of all field class names to the actual class objects */
		static std::unordered_map<std::string, FFieldClass*>& GetNameToFieldClassMap();

		explicit FFieldClass(const char* InCPPName, uint64_t InId, uint64_t InCastFlags, FFieldClass* InSuperClass, FField* (*ConstructFnPtr)(const FFieldVariant&, const std::string&, EObjectFlags));
		~FFieldClass();

		inline const std::string& GetName() const
		{
			return m_Name;
		}
		
		inline uint64_t GetId() const
		{
			return m_Id;
		}
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
	 * Base class of reflection data objects.
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
		void Rename(const std::string& NewName);

		//static FFieldClass* StaticClass();
	};
}