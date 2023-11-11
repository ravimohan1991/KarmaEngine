/**
 * @file GameViewportClient.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UGameViewportClient..
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
	 * @brief A game viewport (FViewport) is a high-level abstract interface for the
	 * platform specific rendering, audio, and input subsystems.
	 *
	 * GameViewportClient is the engine's interface to a game viewport.
	 * Exactly one GameViewportClient is created for each instance of the game.  The
	 * only case (so far) where you might have a single instance of Engine, but
	 * multiple instances of the game (and thus multiple GameViewportClients) is when
	 * you have more than one PIE window running.
	 *
	 * Responsibilities:
	 * 1. propagating input events to the global interactions list
	 */
	class UGameViewportClient : public UObject
	{
		DECLARE_KARMA_CLASS(UGameViewportClient, UObject)

	protected:
		/* The relative world context for this viewport */
		class UWorld* m_World;
	};
}
