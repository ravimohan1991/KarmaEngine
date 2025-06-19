/**
 * @file GameInstance.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UGameInstance..
 * @version 1.0
 * @date September 12, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "krpch.h"

#include "Object.h"

namespace Karma
{
	/**
	 * @brief A high-level manager object for an instance of the running game.
	 *
	 * Spawned at game creation and not destroyed until game instance is shut down.
	 * 
	 * @remark Running as a standalone game, there will be one of these.
	 * @remark Running in PIE (play-in-editor) will generate one of these per PIE instance.
	 */
	class UGameInstance : public UObject
	{
		DECLARE_KARMA_CLASS(UGameInstance, UObject)

	public:
		/**
		 * @brief Constructor
		 *
		 * @since Karma 1.0.0
		 */
		UGameInstance();

	protected:
		struct FWorldContext* m_WorldContext;

	protected:
		/**
		 * @brief Virtual function to allow custom GameInstances an opportunity to set up what it needs
		 *
		 * @since Karma 1.0.0
		 */
		virtual void Init();

		/**
		 * @brief Getter for KEngine object
		 *
		 * @since Karma 1.0.0
		 */
		class KEngine* GetEngine() const;

	public:
		/**
		 * @brief Called to initialize the game instance for standalone instances of the game
		 *
		 * @param InWorldName					The name of the new world
		 * @param InWorldPackage				The container of the new world
		 *
		 * @since Karma 1.0.0
		 */
		void InitializeStandalone(const std::string& InWorldName = "No_Name", UPackage* InWorldPackage = nullptr);

		/**
		 * @brief Getter for FWorldContext instance
		 *
		 * @since Karma 1.0.0
		 */
		struct FWorldContext* GetWorldContext() const { return m_WorldContext; }
	};
}
