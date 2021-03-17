#include "VulkanShader.h"
#include "SPIRV/GlslangToSpv.h"
#include "StandAlone/DirStackFileIncluder.h"

namespace Karma
{
	VulkanShader::VulkanShader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		std::string vString = ReadFile(vertexSrc);
		vertSpirV = Compile(vertexSrc, vString, EShLangVertex);// vertex shader

		vString = ReadFile(fragmentSrc);
		fragSpirV = Compile(fragmentSrc, vString, EShLangFragment);// fragment shader
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
		std::string Path = GetFilePath(src);
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

	std::string VulkanShader::GetFilePath(const std::string& str)
	{
		size_t found = str.find_last_of("/\\");
		return str.substr(0, found);
	}

	std::string VulkanShader::ReadFile(const std::string& file)
	{
		std::string result;
		std::ifstream in(file, std::ios::in, std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			KR_CORE_ASSERT(false, "Could not open shader file " + file);
		}

		return result;
	}

	void VulkanShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{}
}