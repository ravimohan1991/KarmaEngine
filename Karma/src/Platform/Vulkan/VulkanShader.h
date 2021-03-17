#pragma once

#include "Karma/Renderer/Shader.h"
#include "glslang/Public/ShaderLang.h"
#include <string>
#include <map>

namespace Karma
{
	class KARMA_API VulkanShader : public Shader
	{
		enum Vkenum
		{
			VK_VERTEX_SHADER = 0,
			VK_FRAGMENT_SHADER
		};

	public:
		VulkanShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~VulkanShader() override;

		virtual void Bind() const override;
		virtual void UnBind() const override;

		std::vector<uint32_t> Compile(const std::string& src, const std::string& source, EShLanguage lang);

		std::string ReadFile(const std::string& file);
		std::string GetFilePath(const std::string& str);
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

		//Getters
		const std::vector<uint32_t>& GetVertSpirV() const { return vertSpirV; }
		const std::vector<uint32_t>& GetFragSpirV() const { return fragSpirV; }

	private:
		std::vector<uint32_t> vertSpirV;
		std::vector<uint32_t> fragSpirV;
	};

}