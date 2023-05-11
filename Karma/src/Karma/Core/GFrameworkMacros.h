#pragma once

#include "krpch.h"

// https://stackoverflow.com/questions/35111049/using-boost-preprocessor-for-token-comparison
// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...) PROBE(~)
#define COMPARE(x, y) IS_PAREN ( ## x (  ## y) (()) )

/*-----------------------------------------------------------------------------
	Class declaration macros.
-----------------------------------------------------------------------------*/

#define DECLARE_KARMA_CLASS(TClass, TSuperClass) \
public: \
	/** Typedef for the base class ({{ typedef-type }}) */ \
	typedef TSuperClass Super;\
	/** Returns a UClass object representing this class at runtime */ \
	inline static UClass* StaticClass() \
	{ \
		UClass* returnClass = nullptr; \
		if(#TClass !=  #TSuperClass) \
		{ \
			GetPrivateStaticClassBody( \
				"GeneralPackage", \
				#TClass, \
				returnClass, \
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
				returnClass, \
				sizeof(UObject), \
				alignof(UObject), \
				&TClass::NullPointerFunction \
			); \
		} \
		return returnClass; \
	} \
	inline static UClass* NullPointerFunction() \
	{ \
		return nullptr; \
	}