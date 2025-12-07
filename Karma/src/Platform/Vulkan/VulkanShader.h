/**
 * @file VulkanShader.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains VulkanShader class which contains Vulkan specific implementation of Shader class.
 * @version 1.0
 * @date March 9, 2021
 * 
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/Shader.h"
#include "glslang/Public/ShaderLang.h"
#include "Karma/KarmaUtilities.h"

namespace Karma
{
	struct VulkanUniformBuffer;

	/**
	 * @brief Vulkan specific implementation of Shader class.
	 * 
	 * This class handles the compilation of GLSL shader source code to SPIR-V bytecode and manages the associated uniform buffer object.
	 * 
	 * @since Karma 1.0.0
	 */
	class KARMA_API VulkanShader : public Shader
	{
		/**
		 * @brief Enum for Vulkan shader stages
		 * 
		 * Used to identify shader stages during compilation and pipeline creation.
		 * 
		 * @since Karma 1.0.0
		 */
		enum Vkenum
		{
			/** @brief Vertex shader */
			VK_VERTEX_SHADER = 0,

			/** @brief Fragment shader */
			VK_FRAGMENT_SHADER
		};

	public:

		/**
		 * @brief Constructor that compiles vertex and fragment shaders from source code.
		 * 
		 * GLSLANG is used to compile the GLSL source code into SPIR-V bytecode for Vulkan.
		 * 
		 * @param vertexSrc					Path to vertex shader source file
		 * @param fragmentSrc				Path to fragment shader source file
		 * @param ubo						Shared pointer to the uniform buffer object
		 * 
		 * @see Shader::Create()
		 * @since Karma 1.0.0
		 */
		VulkanShader(const std::string& vertexSrc, const std::string& fragmentSrc, std::shared_ptr<UniformBufferObject> ubo);
		
		/**
		 * @brief Destructor
		 * 
		 * Cleans up any allocated resources.
		 * 
		 * @since Karma 1.0.0
		 */
		virtual ~VulkanShader() override;

		virtual void Bind() const override;
		virtual void UnBind() const override;

		/**
		 * @brief Compiles GLSL source code to SPIR-V bytecode using GLSLANG.
		 * 
		 * @param src						Path to the (vertex or fragment) shader GLSL source file
		 * @param source					The actual GLSL source code as a string (read from the file)
		 * 
		 * @param lang						Shader stage (vertex or fragment)
		 * 
		 * @return A vector containing the compiled SPIR-V bytecode
		 * @since Karma 1.0.0
		 */
		std::vector<uint32_t> Compile(const std::string& src, const std::string& source, EShLanguage lang);

		/**
		 * @brief Uploads a 4x4 matrix uniform to the shader.
		 * 
		 * @note For legacy purposes. Vulkan UBO upload is handled in VulkanUniformBuffer::UploadUniformBuffer()
		 * @since Karma 1.0.0
		 */
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

		//Getters
		const std::vector<uint32_t>& GetVertSpirV() const { return vertSpirV; }
		const std::vector<uint32_t>& GetFragSpirV() const { return fragSpirV; }
		std::shared_ptr<VulkanUniformBuffer> GetUniformBufferObject() const { return m_UniformBufferObject; }

	private:
		std::vector<uint32_t> vertSpirV;
		std::vector<uint32_t> fragSpirV;
		std::shared_ptr<VulkanUniformBuffer> m_UniformBufferObject;
	};

}
