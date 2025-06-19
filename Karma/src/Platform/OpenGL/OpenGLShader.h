/**
 * @file OpenGLShader.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains OpenGLShader class
 * @version 1.0
 * @date December 25, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Renderer/Shader.h"
#include "glad/glad.h"
#include "Karma/KarmaUtilities.h"

namespace Karma
{
	struct OpenGLUniformBuffer;

	/**
	 * @brief OpenGL variant of Karma's Shader
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API OpenGLShader : public Shader
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * Creates Karma Shader using OpenGL API with given vertex shader and fragment shader source.
		 *
		 * @param vertexSrc								The source of vertex shader
		 * @param fragmentSrc							The source of fragment shader
		 *
		 * @note The compilation and linking is done by OpenGL
		 * @since Karma 1.0.0
		 */
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);

		/**
		 * @brief Overloaded constructor
		 *
		 * Creates Karma Shader using OpenGL API with given vertex shader, fragment shader, uniform buffer, and shader name
		 *
		 * @param vertexScrFile								The source of vertex shader
		 * @param fragmentScrFile							The source of fragment shader
		 * @param ubo										The list of uniform buffer objects
		 * @param shaderName								The name of the shader
		 *
		 * @since Karma 1.0.0
		 */
		OpenGLShader(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, std::shared_ptr<UniformBufferObject> ubo,
			const std::string& shaderName);

		/**
		 * @brief Deletes the appropriate resources, no longer in use, and clean up
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~OpenGLShader() override;

		/**
		 * @brief Binds the shader and uniform. Seems legacy.
		 *
		 * @todo See to remove the "texSampler" and make generic
		 * @see Bind(const std::string& texShaderNames)
		 * @since Karma 1.0.0.
		 */
		virtual void Bind() const override;

		/**
		 * @brief Bind the shader and specified uniform
		 *
		 * @param texShaderName								The name of the uniform
		 *
		 * @todo Maybe make arrangements for list of uniforms and rename texShaderName
		 * @since Karma 1.0.0
		 */
		virtual void Bind(const std::string& texShaderName) const override;

		/**
		 * @brief Unbind the shader
		 *
		 * @todo Ponder about uniform unbinding
		 * @since Karma 1.0.0
		 */
		virtual void UnBind() const override;

		/**
		 * @brief A legacy uniform upload routine
		 *
		 * @param name					The name of the uniform
		 * @param matrix				The matrix with latest value entries
		 *
		 * @since Karma 1.0.0
		 */
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		/**
		 * @brief A routine to comple and link the shader (both vertex and fragment) for Karma and cache the generated ID to m_RendererID
		 *
		 * @param shaderSources						A list of shader sources to be compiled
		 * @since Karma 1.0.0
		 */
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	private:
		// OpenGL's identification scheme
		uint32_t m_RendererID;
		std::shared_ptr<OpenGLUniformBuffer> m_UniformBufferObject;
	};
}
