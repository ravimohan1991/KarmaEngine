/**
 * @file MeshComponent.h
 * @brief Declaration of the MeshComponent class, representing a PrimitiveComponent that handles mesh rendering in the game engine.
 * @author Ravi Mohan (the_cowboy)
 * @date December 9, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once
#include "Components/PrimitiveComponent.h"

namespace Karma
{
	/**
	 * @brief A component that represents a mesh in the game engine, responsible for rendering and managing mesh data.
	 * 
	 * MeshComponent is an abstract base for any component that is an instance of a renderable collection of triangles.
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API UMeshComponent : public UPrimitiveComponent
	{
		DECLARE_KARMA_CLASS(UPrimitiveComponent, UMeshComponent)

	};
}