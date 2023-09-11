#pragma once

#include "krpch.h"

#include "Object.h"

namespace Karma
{
	/**
	 * Base class of all Engine classes, responsible for management of systems critical to editor or game systems.
	 * Also defines default classes for certain engine systems.
	 *
	 * ATM we have only this class. In future we may subclass to EditorEngine and GameEngine subclasses
	 */
	class KARMA_API KEngine: public UObject
	{
		DECLARE_KARMA_CLASS(KEngine, UObject)

	public:
		/** Update everything.  Should be economic for processor and rest of the resources. */
		virtual void Tick(float DeltaSeconds, bool bIdleMode);

		/** Clean up the GameViewport */
		void CleanupGameViewport();
	};

	/** Global engine pointer. Can be 0 so don't use without checking. */
	extern KEngine*			GEngine;
}
