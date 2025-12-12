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
#include "Components/StaticMeshComponent.h"

namespace Karma
{

	/**
	 * @brief An actor that contains a static mesh component, allowing the mesh to be rendered in the game world.
	 * 
	 * @since Karma 1.0.0
	 */
	class KARMA_API AStaticMeshActor : public AActor
	{
		DECLARE_KARMA_CLASS(AActor, AStaticMeshActor)

	private:
		/**
		 * @brief The static mesh component associated with this actor for rendering
		 * 
		 * @note This is analogous to Unreal Engine's AStaticMeshActor::StaticMeshComponent
		 */
		//std::shared_ptr<class UStaticMeshComponent> m_StaticMeshComponent;
		UStaticMeshComponent* m_StaticMeshComponent;

	public:
		/**
		 * @brief Constructor for StaticMeshActor
		 * 
		 * @param name The name of the static mesh actor.
		 */
		AStaticMeshActor(const std::string& name = "StaticMeshActor");

		/**
		 * @brief Loads a static mesh from a file and assigns it to the static mesh component.
		 * 
		 * @param filePath								The file path of the static mesh to load.
		 * 
		 * @since Karma 1.0.0
		 */
		void LoadMeshFromFile(const std::string& filePath);

		/**
		 * @brief Getter for the static mesh component
		 * 
		 * @return UStaticMeshComponent* The static mesh component of this actor.
		 * 
		 * @since Karma 1.0.0
		 */
		UStaticMeshComponent* GetStaticMeshComponent() const { return m_StaticMeshComponent; }
		
		/**
		 * @brief Destructor for StaticMeshActor
		 */
		virtual ~AStaticMeshActor();
		// Additional methods and members specific to StaticMeshActor can be added here.
	};
}