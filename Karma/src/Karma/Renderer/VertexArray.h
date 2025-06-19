/**
 * @file VertexArray.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the VertexArray class.
 * @version 1.0
 * @date December 24, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Renderer/Buffer.h"
#include "Karma/Renderer/Shader.h"
#include "Karma/Renderer/Mesh.h"
#include "Karma/Renderer/Material.h"

namespace Karma
{
	/**
	 * @brief A class, comprising of Mesh and Material substances along with relevant setup, for a renderable unit
	 */
	class KARMA_API VertexArray
	{
	public:
		/**
		 * @brief A destructor
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~VertexArray()
		{
		}

		/**
		 * @brief For binding OpenGL vertex array object
		 *
		 * @todo Seems only specific to OpenGL. Need to think the abstraction with Vulkan in mind
		 * @since Karma 1.0.0
		 */
		virtual void Bind() const = 0;

		/**
		 * @brief Undo what Bind does
		 *
		 * @todo Again, need to think the abstraction
		 * @since Karma 1.0.0
		 */
		virtual void UnBind() const = 0;

		// For legacy purposes. Use Mesh abstraction.
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const IndexBuffer* GetIndexBuffer() const = 0;
		// end legacy purpose context

		/**
		 * @brief Sets the index and vertex buffers seperately
		 *
		 * @param mesh					A collection of index and vertex buffers to be set
		 * @since Karma 1.0.0
		 */
		virtual void SetMesh(std::shared_ptr<Mesh> mesh) = 0;

		// For legacy purposes. Use Material abstraction.
		virtual void SetShader(std::shared_ptr<Shader> shader) = 0;

		/**
		 * @brief Sets the material
		 *
		 * @param material				Shader and texture setting
		 * @todo a seperate setting of shader may be redundant
		 *
		 * @since Karma 1.0.0
		 */
		virtual void SetMaterial(std::shared_ptr<Material> material) = 0;

		/**
		 * @brief Called each game loop for updating
		 *
		 * @note includes uniform updation  (vertex coordinates updation in shader by view and projection matrices)
		 * @since Karma 1.0.0
		 */
		virtual void UpdateProcessAndSetReadyForSubmission() const = 0;

		// Getters
		//virtual std::shared_ptr<Mesh> GetMesh() const = 0;

		/**
		 * @brief Getter function for material
		 *
		 * @since Karma 1.0.0
		 */
		virtual std::shared_ptr<Material> GetMaterial() const = 0;

		/**
		 * @brief Create the VertexArray object based on the rendering API chosen by the programmer
		 *
		 * @since Karma 1.0.0
		 */
		static VertexArray* Create();
	};
}
