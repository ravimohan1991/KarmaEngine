/**
 * @file OpenGLVertexArray.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains OpenGLVertexArray class
 * @version 1.0
 * @date December 24, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/VertexArray.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Karma
{
	/**
	 * @brief OpenGL's vertex array class
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API OpenGLVertexArray : public VertexArray
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * Generates vertex array buffer and binds the array
		 *
		 * @since Karma 1.0.0
		 */
		OpenGLVertexArray();

		/**
		 * @brief Destructor to delete resources and clenup
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~OpenGLVertexArray();

		/**
		 * @brief Binds the vertex array object
		 *
		 * @since Karma 1.0.0
		 */
		virtual void Bind() const override;

		/**
		 * @brief Unbinds the array
		 *
		 * @since Karma 1.0.0
		 */
		virtual void UnBind() const override;

		// For legacy purposes. Use Mesh abstraction

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
		// Use Material abstraction
		virtual void SetShader(std::shared_ptr<Shader> shader) override {}

		// End legacy functions

		/**
		 * @brief Using OpenGL's API, the vertex buffer is processed for rendering
		 *
		 * @param mesh								The container for vertex buffer loaded from .obj file
		 *
		 * @since Karma 1.0.0
		 */
		virtual void SetMesh(std::shared_ptr<Mesh> mesh) override;

		/**
		 * @brief Sets the material cache for OpenGL's rendering
		 *
		 * @since Karma 1.0.0
		 */
		virtual void SetMaterial(std::shared_ptr<Material> material) override;

		/**
		 * @brief Getter for vertex buffer
		 *
		 * @since Karma 1.0.0
		 */
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }

		/**
		 * @brief Getter for index buffer
		 *
		 * @since Karma 1.0.0
		 */
		virtual const IndexBuffer* GetIndexBuffer() const override { return m_IndexBuffer.get(); }

		/**
		 * @brief Getter for Material
		 *
		 * @since Karma 1.0.0
		 */
		virtual std::shared_ptr<Material> GetMaterial() const override { return m_Materials.at(0); }

		/**
		 * @brief OpenGL's way of processing Mesh + Material before submitting to GPU (or issuing draw call via RenderCommand::DrawIndexed)
		 *
		 * @since Karma 1.0.0
		 */
		virtual void UpdateProcessAndSetReadyForSubmission() const override;

	private:
		uint32_t m_RendererID;

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		// Material relevant members
		std::vector<std::shared_ptr<Material>> m_Materials;
		std::vector<std::shared_ptr<OpenGLShader>> m_Shaders;
		std::shared_ptr<OpenGLShader> m_Shader;
	};
}
