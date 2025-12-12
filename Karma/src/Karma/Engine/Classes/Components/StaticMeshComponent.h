/**
 * @file StaticMeshComponent.h
 * @brief Declaration of the StaticMeshComponent class, representing a component that handles static mesh rendering in the game engine.
 * @author Ravi Mohan (the_cowboy)
 * @date December 9, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "Components/MeshComponent.h"
#include "Renderer/Mesh.h"

namespace Karma
{
	/**
	 * @brief A component that represents a static mesh in the game engine, responsible for rendering and managing static mesh data.
	 * 
	 * StaticMeshComponent is used to render a static mesh asset in the world. It is a subclass of MeshComponent and provides functionality specific to static meshes.
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API UStaticMeshComponent : public UMeshComponent
	{
		DECLARE_KARMA_CLASS(UMeshComponent, UStaticMeshComponent)

	private:

		/**
		 * @brief The static mesh asset associated with this component for rendering
		 * 
		 * @note This is analogous to Unreal Engine's UStaticMeshComponent::StaticMesh
		 */
		std::shared_ptr<Mesh> m_StaticMesh;

	public:
		/**
		 * @brief Constructor for StaticMeshComponent
		 * 
		 * @param name The name of the static mesh component.
		 */
		UStaticMeshComponent(const std::string& name = "StaticMeshComponent");
		
		/**
		 * @brief Destructor for StaticMeshComponent
		 */
		virtual ~UStaticMeshComponent();
		// Additional methods and members specific to StaticMeshComponent can be added here.

		/**
		 * @brief Sets the static mesh associated with this component.
		 * 
		 * @param staticMesh							The static mesh to associate with this component.
		 * @since Karma 1.0.0
		 */
		void SetStaticMesh(std::shared_ptr<Mesh> staticMesh) { m_StaticMesh = staticMesh; }

		/**
		 * @brief Gets the static mesh associated with this component.
		 * 
		 * @return std::shared_ptr<class Mesh> The static mesh associated with this component.
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<Mesh> GetStaticMesh() const { return m_StaticMesh; }
	};
}