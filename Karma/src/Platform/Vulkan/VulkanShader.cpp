#include "VulkanShader.h"
#include "SPIRV/GlslangToSpv.h"
#include "StandAlone/DirStackFileIncluder.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Karma
{
	VulkanShader::VulkanShader(const std::string& vertexSrc, const std::string& fragmentSrc, std::shared_ptr<UniformBufferObject> ubo) : Shader(ubo)
	{
		std::string vString = KarmaUtilities::ReadFileToSpitString(vertexSrc);
		vertSpirV = Compile(vertexSrc, vString, EShLangVertex);// vertex shader

		vString = KarmaUtilities::ReadFileToSpitString(fragmentSrc);
		fragSpirV = Compile(fragmentSrc, vString, EShLangFragment);// fragment shader

		m_UniformBufferObject = std::static_pointer_cast<VulkanUniformBuffer>(ubo);
	}

	VulkanShader::~VulkanShader()
	{

	}

	std::vector<uint32_t> VulkanShader::Compile(const std::string& src, const std::string& source, EShLanguage lang)
	{
		KR_CORE_INFO("Compiling {0} {1} for Vulkan ...", lang == EShLangVertex ? "vertex shader" : "fragment shader", src);

		const char* sString = source.c_str();
		glslang::TShader Shader(lang);
		Shader.setStrings(&sString, 1);

		int ClientInputSemanticsVersion = 100;
		glslang::EshTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
		glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

		Shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, ClientInputSemanticsVersion);
		Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
		Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

		TBuiltInResource Resources{};
		Resources.maxDrawBuffers = true;

		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

		const int DefaultVersion = 100;

		DirStackFileIncluder Includer;
		std::string Path = KarmaUtilities::GetFilePath(src);
		Includer.pushExternalLocalDirectory(Path);

		std::string PreprocessedGLSL;
		if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
		{
			KR_CORE_ERROR("{0}", Shader.getInfoLog());
			KR_CORE_ERROR("{0}", Shader.getInfoDebugLog());
			KR_CORE_ASSERT(false, "Shader preprocessing failed!");
		}

		const char* PreprocessedCStr = PreprocessedGLSL.c_str();
		Shader.setStrings(&PreprocessedCStr, 1);

		if (!Shader.parse(&Resources, 100, false, messages))
		{
			KR_CORE_ERROR("{0}", Shader.getInfoLog());
			KR_CORE_ERROR("{0}", Shader.getInfoDebugLog());
			KR_CORE_ASSERT(false, "GLSL parsing failed!");
		}

		glslang::TProgram Program;
		Program.addShader(&Shader);

		if (!Program.link(messages))
		{
			KR_CORE_ERROR("{0}", Shader.getInfoLog());
			KR_CORE_ERROR("{0}", Shader.getInfoDebugLog());
			KR_CORE_ASSERT(false, "Shader link faliure!");
		}

		std::vector<uint32_t> SpirV;
		spv::SpvBuildLogger Logger;
		glslang::SpvOptions SpvOptions;
		glslang::GlslangToSpv(*Program.getIntermediate(lang), SpirV, &Logger, &SpvOptions);

		return SpirV;
	}

	void VulkanShader::Bind() const
	{}

	void VulkanShader::UnBind() const
	{}

	void VulkanShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{}
}
