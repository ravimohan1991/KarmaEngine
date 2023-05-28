#pragma once

#include "krpch.h"

/*-----------------------------------------------------------------------------
	Class declaration macros.
-----------------------------------------------------------------------------*/

#define DECLARE_KARMA_CLASS(TClass, TSuperClass) \
public: \
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
				&TClass::Super::NullClass \
			); \
		} \
		return returnClass_##TClass; \
	} \
	inline static UClass* NullClass() \
	{ \
		return nullptr; \
	}
