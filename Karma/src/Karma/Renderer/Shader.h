/**
 * @file Shader.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the Shader class.
 * @version 1.0
 * @date December 21, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Renderer/Buffer.h"
#include "glm/glm.hpp"

namespace Karma
{
	/**
	 * @brief Base class of vertex + fragment shaders (for Vulkan and OpenGL)
	 */
	class KARMA_API Shader
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param ubo						Uniform buffer object to a assigned
		 * @since Karma 1.0.0
		 */
		Shader(std::shared_ptr<UniformBufferObject> ubo) : m_UniformBufferObject(ubo)
		{}

		/**
		 * @brief Destructor
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~Shader() = default;
		
		/**
		 * @brief Installs a shader (vertex + fragment) and uniform program object as part of current rendering state
		 *
		 * @todo For OpenGL only. Vulkan Bind() is empty and may need abstract thinking
		 * @since Karma 1.0.0
		 */
		virtual void Bind() const {}

		/**
		 * @brief Binding using name of texture
		 *
		 * @todo Think with the previous binding routiine
		 * @since Karma 1.0.0
		 */
		virtual void Bind(const std::string& texShaderNames) const {}

		/**
		 * @brief Undo the binding of shader program
		 *
		 * @todo Only OpenGL uses this, that too, for shader and not uniform. May need pondering over.
		 * @since Karma 1.0.0
		 */
		virtual void UnBind() const {}

		/**
		 * @brief Getter for uniform buffer object
		 *
		 * @return pointer to UniformBufferObject present in the shader
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<UniformBufferObject> GetUniformBufferObject() const { return m_UniformBufferObject; }

		/**
		 * @brief Instantiating shader object according to the programmer chosen API
		 *
		 * @deprecated Use the overloaded function with uniformbuffer object argument
		 * @since Karma 1.0.0
		 */
		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);

		/**
		 * @brief For instantiating shader object accodring to programer's chosen API
		 *
		 * @param vertexSrcFile					Path to vertex shader (filename included). For instance "../Resources/Shaders/shader.vert", relative to
		 * 										Engine's running directory
		 * @param fragmentSrcFile				Path to fragment shader (filename included).
		 * @param ubo							UniformBufferObject to be used
		 * @param shaderName					Name of the shader to be supplied
		 * @since Karma 1.0.0
		 */
		static Shader* Create(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, std::shared_ptr<UniformBufferObject> ubo,
			const std::string& shaderName = "NoNamedShader");

		// Getters
		/**
		 * @brief Getter for shader name
		 *
		 * @note Could be a pointer?
		 * @since Karma 1.0.0
		 */
		const std::string& GetShaderName() const { return m_ShaderName; }

	private:
		std::shared_ptr<UniformBufferObject> m_UniformBufferObject;
	
	protected:
		std::string m_ShaderName;
	};
}
