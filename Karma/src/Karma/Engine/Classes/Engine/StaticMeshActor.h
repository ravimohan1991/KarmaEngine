/**
 * @file StaticMeshActor.h
 * @brief Declaration of the StaticMeshActor class, representing an actor with a static mesh component in the game world.
 * @author Ravi Mohan (the_cowboy)
 * @date December 7, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "GameFramework/Actor.h"

namespace Karma
{
	/**
	 * @brief An actor that contains a static mesh component, allowing it to be rendered in the game world.
	 * 
	 * @since Karma 1.0.0
	 */
	class KARMA_API AStaticMeshActor : public AActor
	{
		DECLARE_KARMA_CLASS(AActor, AStaticMeshActor)

	private:


	public:
		/**
		 * @brief Constructor for StaticMeshActor
		 * 
		 * @param name The name of the static mesh actor.
		 */
		AStaticMeshActor(const std::string& name = "StaticMeshActor");
		
		/**
		 * @brief Destructor for StaticMeshActor
		 */
		virtual ~AStaticMeshActor();
		// Additional methods and members specific to StaticMeshActor can be added here.
	};
}