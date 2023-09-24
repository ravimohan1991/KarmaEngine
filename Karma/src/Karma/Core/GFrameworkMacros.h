/**
 * @file GFrameWorkMacros.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains some commonly used game code macros.
 * @version 1.0
 * @date May 10, 2023
 *
 * @copyright Karma Engine: copyright(c) People of India, in accordance with Epic EULA (https://www.unrealengine.com/en-US/eula/unreal)
 */

#pragma once

#include "krpch.h"

enum EInternal						{EC_InternalUseOnlyConstructor};
typedef void		(*ClassConstructorType)				(const Karma::FObjectInitializer&);

#define DEFINE_DEFAULT_CONSTRUCTOR_CALL(TClass) \
	static void __DefaultConstructor(const FObjectInitializer& X) { new((EInternal*)X.GetObj())TClass; }

#define DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(TClass) \
	static void __DefaultConstructor(const FObjectInitializer& X) { new((EInternal*)X.GetObj())TClass(X); }

/*-----------------------------------------------------------------------------
	Class declaration macros.
-----------------------------------------------------------------------------*/

#define DECLARE_KARMA_CLASS(TClass, TSuperClass) \
public: \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(TClass) \
	/** Typedef for the base class ({{ typedef-type }}) */ \
	typedef TSuperClass Super;\
	/** Returns a UClass object representing this class at runtime */ \
	static UClass* StaticClass() \
	{ \
		UClass* returnClass_##TClass = nullptr; \
		if(strcmp(#TClass, #TSuperClass) != 0) \
		{ \
			GetPrivateStaticClassBody( \
				"GeneralPackage", \
				#TClass, \
				returnClass_##TClass, \
				sizeof(TClass), \
				alignof(TClass), \
				(ClassConstructorType)InternalConstructor<TClass>, \
				&TClass::Super::StaticClass \
			); \
		} \
		else \
		{ \
			GetPrivateStaticClassBody( \
				"GeneralPackage", \
				"UObject", \
				returnClass_##TClass, \
				sizeof(UObject), \
				alignof(UObject), \
				(ClassConstructorType)InternalConstructor<TClass>, \
				&TClass::Super::NullClass \
			); \
		} \
		return returnClass_##TClass; \
	} \
	inline static UClass* NullClass() \
	{ \
		return nullptr; \
	}
