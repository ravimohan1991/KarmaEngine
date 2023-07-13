#pragma once

#include "krpch.h"

#include "UObjectBase.h"
#include "GFrameworkMacros.h"

namespace Karma
{
	class UClass;

	typedef UClass* (*StaticClassFunctionType)();

	/**
	 * Helper template to call the default constructor for a class
	 */
	template<class T>
	void InternalConstructor(const FObjectInitializer& X)
	{
		T::__DefaultConstructor(X);
	}

	/**
	 * Helper template allocate and construct a UClass
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

	KARMA_API void InitializePrivateStaticClass(
		class UClass* TClass_Super_StaticClass,
		class UClass* TClass_PrivateStaticClass,
		class UClass* TClass_WithinClass_StaticClass,
		const std::string& PackageName,
		const std::string& Name
	);

	class KARMA_API UObject : public UObjectBase
	{
		// In UE, this is done in ObjectMacros.h, #define DECLARE_CLASS
		DECLARE_KARMA_CLASS(UObject, UObject)

	private:
		//UClass* m_StaticClass;

	public:
		UObject();
		UObject(UClass* inClass, EObjectFlags inFlags, EInternalObjectFlags inInternalFlags, UObject* inOuter, const std::string& inName);

		static const char* StaticConfigName()
		{
			return "Engine";
		}

		/**
		 * Called to finish destroying the object.  After UObject::FinishDestroy is called, the object's memory should no longer be accessed.
		 *
		 * @warning Because properties are destroyed here, Super::FinishDestroy() should always be called at the end of your child class's FinishDestroy() method, rather than at the beginning.
		 */
		virtual void FinishDestroy();

		/** Return a one line description of an object for viewing in the thumbnail view of the generic browser */
		virtual const std::string& GetDesc() { static std::string someString = "";  return someString; }

	public:
		/**
		 * Returns what UWorld this object is contained within.
		 * By default this will follow its Outer chain, but it should be overridden if that will not work.
		 */
		virtual class UWorld* GetWorld() const;

		/**
		 * Test validity of object similar to IsValid(Test) however the null pointer test is skipped
		 *
		 * @param	Test			The object to test
		 * @return	Return true if the object is usable: not pending kill or garbage
		 */
		bool IsValidChecked(const UObject* Test);

		/**
		 * Called before destroying the object.  This is called immediately upon deciding to destroy the object, to allow the object to begin an
		 * asynchronous cleanup process.
		 */
		virtual void BeginDestroy();
	};
}
