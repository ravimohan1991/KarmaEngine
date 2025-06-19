/**
 * @file OpenGLBuffer.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains OpenGLVertex/Index/ImageBuffer classes.
 * @version 1.0
 * @date December 21, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/Buffer.h"

namespace Karma
{
	/**
	 * @brief OpenGL's vertex buffer class, used in agnostic Mesh instance.
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		/**
		 * @brief Constructor
		 *
		 * @param vertices						float array of interleaved vertex data (including position, uv, color, normal, and tangent) based on the BufferLayout
		 * @param size							Size (in bytes) of the vertex buffer (number of mesh vertices * sum of each vertex attribute's size). For instance:
		 *										@code{}
		 * 											float vertices[3 * 7] = {
		 * 														-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 * 														0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 * 														0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		 * 											};
		 *										@endcode
		 * 										will have size = 3 * (7 * sizeof(float)).
		 *
		 * @see Mesh::DealVertexIndexBufferData
		 * @since Karma 1.0.0
		 */
		OpenGLVertexBuffer(float* vertices, uint32_t size);

		/**
		 * @brief Destructor
		 *
		 * Deletes the buffers and clears up OpenGL relevant resources
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~OpenGLVertexBuffer();

		/**
		 * @brief Binds the vertexbuffer
		 *
		 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindBuffer.xhtml
		 * @since Karma 1.0.0
		 */
		virtual void Bind() const override;

		/**
		 * @brief UnBinds the vertexbuffer (cleanup)
		 *
		 * @since Karma 1.0.0
		 */
		virtual void UnBind() const override;

		/**
		 * @brief Getter for the layout of the vertex buffer
		 *
		 * @since Karma 1.0.0
		 */
		virtual const BufferLayout& GetLayout() const override
		{
			return m_Layout;
		}

		/**
		 * @brief Sets the layout of the vertexbuffer
		 *
		 * @param layout						The reference to layout to be set
		 *
		 * @since Karma 1.0.0
		 */
		virtual void SetLayout(const BufferLayout& layout) override;

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	/**
	 * @brief OpenGL's index buffer class, used in agnostic Mesh instance
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		/**
		 * @brief Constructor
		 *
		 * Initializes the relevant buffers and uploads them to GPU.
		 *
		 * @param indices						The array of floats containing indexbuffer data
		 * @param count							The length of the indexbuffer array
		 *
		 * @since Karma 1.0.0
		 */
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);

		/**
		 * @brief Deletes the buffers and cleans up
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~OpenGLIndexBuffer();

		/**
		 * @brief Binds the indexbuffer
		 *
		 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindBuffer.xhtml
		 * @since Karma 1.0.0
		 */
		virtual void Bind() const override;

		/**
		 * @brief UnBinds the buffer
		 *
		 * @since Karma 1.0.0
		 */
		virtual void UnBind() const override;

		/**
		 * @brief Returns the length of indexbuffer
		 *
		 * @since Karma 1.0.0
		 */
		virtual uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

	/**
	 * @brief OpenGL's image buffer, used in agnostic Texture instance.
	 */
	class KARMA_API OpenGLImageBuffer : public ImageBuffer
	{
	public:
		/**
		 * @brief Sets up the resources for supplied texture
		 *
		 * @param fileName						The path to the file containing texture
		 *
		 * @see KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateTexture
		 * @since Karma 1.0.0
		 */
		static void SetUpImageBuffer(const char* filename);

		/**
		 * @brief Bind the image buffer
		 *
		 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindTexture.xhtml
		 * @since Karma 1.0.0
		 */
		static void BindTexture();// Experimental, need right OpenGLTexture abstraction

	private:
		static unsigned int m_ImageBufferID;
	};

	/**
	 * @brief OpenGL's uniform buffer, used in OpenGLShader type instance of Shader object
	 */
	struct KARMA_API OpenGLUniformBuffer : public UniformBufferObject
	{
	public:
		/**
		 * @brief Constructor
		 * Generate the buffers for uniforms and bind them
		 *
		 * @param dataTypes								The vector of ShaderDataType. See \ref UniformBufferObject::Create for information
		 *
		 * @since Karma 1.0.0
		 */
		OpenGLUniformBuffer(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex);

		/**
		 * @brief Getter for the uniform id
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t GetUniformsID() const { return m_UniformsID; }

		/**
		 * @brief Delete the buffers and clean up the resources appropriately
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~OpenGLUniformBuffer();

		/**
		 * @brief Upload the uniform to GPU
		 *
		 * @since Karma 1.0.0
		 */
		virtual void UploadUniformBuffer(size_t frameIndex) override;

	private:
		/**
		 * @brief Generate the uniform buffers
		 *
		 * @since Karma 1.0.0
		 */
		void GenerateUniformBufferObject();

		/**
		 * @brief Bind the buffer objects
		 *
		 * @since Karma 1.0.0
		 */
		void BindUniformBufferObject() const;

	private:
		uint32_t m_UniformsID;
	};
}
