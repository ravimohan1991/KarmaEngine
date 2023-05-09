#pragma once

/*-----------------------------------------------------------------------------
	Class declaration macros.
-----------------------------------------------------------------------------*/

#define DECLARE_KARMA_CLASS(TClass, TSuperClass) \
public: \
	/** Typedef for the base class ({{ typedef-type }}) */ \
	typedef TSuperClass Super;