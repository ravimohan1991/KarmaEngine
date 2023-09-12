#pragma once

#include "krpch.h"

#include "Object.h"

namespace Karma
{
	/**
	 * GameInstance: high-level manager object for an instance of the running game.
	 * Spawned at game creation and not destroyed until game instance is shut down.
	 * Running as a standalone game, there will be one of these.
	 * Running in PIE (play-in-editor) will generate one of these per PIE instance.
	 */
	class UGameInstance : public UObject
	{
		DECLARE_KARMA_CLASS(UGameInstance, UObject)

	protected:
		struct FWorldContext* m_WorldContext;

	protected:
		/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
		virtual void Init();

		class KEngine* GetEngine() const;

	public:
		/* Called to initialize the game instance for standalone instances of the game */
		void InitializeStandalone(const std::string& InWorldName = "No_Name", UPackage* InWorldPackage = nullptr);

		struct FWorldContext* GetWorldContext() const { return m_WorldContext; }

	};
}
