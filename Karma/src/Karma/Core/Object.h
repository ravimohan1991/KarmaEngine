#pragma once

#include "krpch.h"

namespace Karma
{
	class KARMA_API UObject
	{
	public:
		static const TCHAR* StaticConfigName()
		{
			return TEXT("Engine");
		}

		/**
		 * Called to finish destroying the object.  After UObject::FinishDestroy is called, the object's memory should no longer be accessed.
		 *
		 * @warning Because properties are destroyed here, Super::FinishDestroy() should always be called at the end of your child class's FinishDestroy() method, rather than at the beginning.
		 */
		virtual void FinishDestroy();
	};
}