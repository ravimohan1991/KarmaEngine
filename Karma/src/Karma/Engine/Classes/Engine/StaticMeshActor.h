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
	class UniformBufferObject;

	/**
	 * @brief An actor that contains a static mesh component, allowing the mesh to be rendered in the game world.
	 * 
	 * @since Karma 1.0.0
	 */
	class KARMA_API AStaticMeshActor : public AActor
	{
		DECLARE_KARMA_CLASS(AStaticMeshActor, AActor)

	private:
		/**
		 * @brief The static mesh component associated with this actor for rendering
		 * 
		 * @note This is analogous to Unreal Engine's AStaticMeshActor::StaticMeshComponent
		 */
		//std::shared_ptr<class UStaticMeshComponent> m_StaticMeshComponent;
		UStaticMeshComponent* m_StaticMeshComponent;
		
		/**
		 * @brief Uniform buffer object for the mesh's transformation matrix
		 * 
		 * Used to upload the actor's transform to the GPU for rendering
		 */
		std::shared_ptr<UniformBufferObject> m_MeshTransformUniform;

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
		 * @brief Getter for the mesh transform uniform buffer object
		 * 
		 * @return std::shared_ptr<UniformBufferObject> The uniform buffer object for the mesh's transformation matrix.
		 * 
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<UniformBufferObject> GetMeshTransformUniform() const { return m_MeshTransformUniform; }
	};
}
