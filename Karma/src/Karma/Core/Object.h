#pragma once

#include "krpch.h"

#include "UObjectBase.h"

namespace Karma
{
	class UClass;

	class KARMA_API UObject : public UObjectBase
	{
	private:
		UClass* m_StaticClass;

	public:
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
		static UClass* StaticClass(UObject* someObject);

		template<typename classType>
		static UClass* StaticClass()
		{
			classType someClass;
			return StaticClass(&someClass);
		}

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
	};
}
