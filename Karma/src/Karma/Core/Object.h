/**
 * @file Object.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UObject along with helper functions.
 * @version 1.0
 * @date February 27, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "UObjectBase.h"
#include "GFrameworkMacros.h"

namespace Karma
{
	class UClass;

	/**
	 * A function pointer with UClass return type.
	 *
	 * @see GetPrivateStaticClassBody()
	 */
	typedef UClass* (*StaticClassFunctionType)();

	/**
	 * @brief Helper template to call the default constructor for a class
	 *
	 * @see #define DEFINE_DEFAULT_CONSTRUCTOR_CALL(TClass) in GFrameworkMacros.h
	 */
	template<class T>
	void InternalConstructor(const FObjectInitializer& X)
	{
		T::__DefaultConstructor(X);
	}

	/**
	 * @brief Helper template to allocate and construct a UClass
	 *
	 * In UE, this helper is declared and defined in Class.h/.cpp. I don't know
	 * how that worked, hehe.
	 *
	 * @param PackageName name of the package this class will be inside
	 * @param Name of the class
	 * @param ReturnClass reference to pointer to result. This must be PrivateStaticClass.
	 * @param RegisterNativeFunc Native function registration function pointer.
	 * @param InSize Size of the class
	 * @param InAlignment Alignment of the class
	 * @param InClassFlags Class flags
	 * @param InClassCastFlags Class cast flags
	 * @param InConfigName Class config name
	 * @param InClassConstructor Class constructor function pointer
	 * @param InClassVTableHelperCtorCaller Class constructor function for vtable pointer
	 * @param InCppClassStaticFunctions Function pointers for the class's version of Unreal's reflected static functions
	 * @param InSuperClassFn Super class function pointer
	 * @param WithinClass Within class
	 *
	 * @see DECLARE_KARMA_CLASS(TClass, TSuperClass) in GFrameworkMacros.h
	 * @todo some params are not functional yet
	 *
	 * @since Karma 1.0.0
	 */
	KARMA_API void GetPrivateStaticClassBody(
		const std::string& PackageName,
		const std::string& Name,
		UClass*& ReturnClass,
		/*void(*RegisterNativeFunc)(),*/
		size_t InSize,
		size_t InAlignment,
		/*EClassFlags InClassFlags,
		EClassCastFlags InClassCastFlags,
		const TCHAR* InConfigName,*/
		ClassConstructorType InClassConstructor,
		/*UClass::ClassVTableHelperCtorCallerType InClassVTableHelperCtorCaller,
		FUObjectCppClassStaticFunctions&& InCppClassStaticFunctions,*/
		StaticClassFunctionType InSuperClassFn
		/*UClass::StaticClassFunctionType InWithinClassFn*/);

	/**
	 * Shared function called from the various InitializePrivateStaticClass functions generated my the DECLARE_KARMA_CLASS macro.
	 * Basically sets up the class hierarchy by setting UStruct::m_SuperStruct with TClass_Super_StaticClass
	 *
	 * @param TClass_Super_StaticClass	The class which is base for this class
	 * @param TClass_PrivateStaticClass	The current class which is being initialized
	 * @see UClass::SetSuperStruct
	 * @since Karma 1.0.0
	 */
	KARMA_API void InitializePrivateStaticClass(
		class UClass* TClass_Super_StaticClass,
		class UClass* TClass_PrivateStaticClass,
		class UClass* TClass_WithinClass_StaticClass,
		const std::string& PackageName,
		const std::string& Name
	);

	/**
	 * @brief The base class of all the game code relevant objects.
	 *
	 * The base class of all UE objects. The type of an object is defined by its UClass.
	 * This provides support functions for creating and using objects, and virtual functions that should be overridden in child classes.
	 *
	 * @see https://docs.unrealengine.com/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Objects
	 */
	class KARMA_API UObject : public UObjectBase
	{
		/*
		 * Declaration shaped for Karma's game code
		 * 
		 * Contains reference to default constructor (defined in this file) and definition
		 * for "Super", the base class and StaticClass()
		 * 
		 * Note: UObject's base class is UObject
		 */
		DECLARE_KARMA_CLASS(UObject, UObject)

	private:
		//UClass* m_StaticClass;

	public:
		/**
		 * Default constructor does nothing for now except well, being used as default constructor in DECLARE_KARMA_CLASS(TClass, TSuperClass) which is needed
		 * for placement new and, thus, initializing the UObject
		 *
		 * @see DEFINE_DEFAULT_CONSTRUCTOR_CALL(TClass)
		 * @see https://forums.unrealengine.com/t/placement-new-for-aactor-spawning/1223044
		 *
		 * @since Karma 1.0.0
		 */
		UObject();

		/**
		 * A legacy function from UE
		 */
		static const char* StaticConfigName()
		{
			return "Engine";
		}

		/**
		 * Called to finish destroying the object.  After UObject::FinishDestroy is called, the object's memory should no longer be accessed.
		 *
		 * @warning Because properties are destroyed here, Super::FinishDestroy() should always be called at the end of your child class's FinishDestroy() method, rather than at the beginning.
		 * @todo To be written with Shiva logic for UObject destruction.
		 *
		 * @since Karma 1.0.0
		 */
		virtual void FinishDestroy();

		/**
		 * @brief Return a one line description of an object for viewing in the thumbnail view of the generic browser
		 * @todo Place holder for now. Plan to rewrite once Editor reaches that stage
		 *
		 * @since Karma 1.0.0
		 */
		virtual const std::string& GetDesc() { static std::string someString = "";  return someString; }

	public:
		/**
		 * @brief Returns what UWorld this object is contained within.
		 *
		 * By default this will follow its Outer chain, but it should be overridden if that will not work.
		 *
		 * @since Karma 1.0.0
		 */
		virtual class UWorld* GetWorld() const;

		/**
		 * Test validity of object similar to IsValid(Test) however the null pointer test is skipped
		 *
		 * @param	Test			The object to test
		 * @return	Return true if the object is usable: not pending kill or garbage
		 *
		 * @since Karma 1.0.0
		 */
		bool IsValidChecked(const UObject* Test);

		/**
		 * @brief Called before destroying the object.  This is called immediately upon deciding to destroy the object, to allow the object to begin an
		 * asynchronous cleanup process.
		 * @todo To be written with Shiva logic for UObject destruction.
		 *
		 * @since Karma 1.0.0
		 */
		virtual void BeginDestroy();

		/**
		 * Note that the object will be modified.  If we are currently recording into the
		 * transaction buffer (undo/redo), save a copy of this object into the buffer and
		 * marks the package as needing to be saved.
		 *
		 * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
		 *								currently recording an active undo/redo transaction
		 * @return true if the object was saved to the transaction buffer
		 *
		 * @since Karma 1.0.0
		 */
#if WITH_EDITOR
		virtual bool Modify(bool bAlwaysMarkDirty = true);

		/** @brief Utility to allow overrides of Modify to avoid doing work if this object cannot be safely modified */
		bool CanModify() const;
#else
		FORCEINLINE bool Modify(bool bAlwaysMarkDirty = true) { return false; }
#endif

		/**
		 * @brief Test the selection state of a UObject
		 *
		 * @return		true if the object is selected, false otherwise.
		 * @todo UE this doesn't belong here, but it doesn't belong anywhere else any better. To be written after Editor is functional with UObjects
		 *
		 * @since Karma 1.0.0
		 */
		bool IsSelected() const;
	};

	/**
	 * @brief Note the name. Need to understand UE's take upon the subject
	 *
	 * @since Karma 1.0.0
	 */

	FORCEINLINE bool TentativeFlagChecks(const UObject* Test)
	{
		KR_CORE_ASSERT(GUObjectStore.IndexToObject(Test->GetInternalIndex())->HasAnyFlags(EInternalObjectFlags(int32_t(EInternalObjectFlags::PendingKill) | int32_t(EInternalObjectFlags::Garbage))) == Test->HasAnyFlags(EObjectFlags(RF_PendingKill | RF_Garbage)), "");

		return !Test->HasAnyFlags(EObjectFlags(RF_PendingKill | RF_Garbage));
	}

	/**
	 * @brief Test validity of object
	 *
	 * @param	Test			The object to test
	 * @return	Return true if the object is usable: non-null and not pending kill or garbage
	 *
	 * @since Karma 1.0.0
	 */
	FORCEINLINE bool IsValid(const UObject* Test)
	{
		return Test && /*FInternalUObjectBaseUtilityIsValidFlagsChecker::CheckObjectValidBasedOnItsFlags(Test)*/ TentativeFlagChecks(Test);
	}
}
