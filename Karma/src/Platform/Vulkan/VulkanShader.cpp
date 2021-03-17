#include "VulkanShader.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "StandAlone/DirStackFileIncluder.h"

namespace Karma
{
	const TBuiltInResource DefaultTBuiltInResource = { };

	VulkanShader::VulkanShader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		std::unordered_map<Vkenum, std::string> shaderSources;
		std::string vString = ReadFile(vertexSrc);
		const char* vertexString = vString.c_str();
		glslang::TShader vertexShader(EShLangVertex);
		vertexShader.setStrings(&vertexString, 1);

		int ClientInputSemanticsVersion = 100;
		glslang::EshTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
		glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

		vertexShader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientVulkan, ClientInputSemanticsVersion);
		vertexShader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
		vertexShader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

		TBuiltInResource Resources;
		Resources = DefaultTBuiltInResource;
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

		const int DefaultVersion = 100;

		DirStackFileIncluder Includer;
		std::string Path = GetFilePath(vertexSrc);
		Includer.pushExternalLocalDirectory(Path);

		std::string PreprocessedGLSL;
		if (!vertexShader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
		{
			KR_CORE_ERROR("{0}", vertexShader.getInfoLog());
			KR_CORE_ERROR("{0}", vertexShader.getInfoDebugLog());
			KR_CORE_ASSERT(false, "Shader preprocessing failed!");
		}

		const char* PreprocessedCStr = PreprocessedGLSL.c_str();
		vertexShader.setStrings(&PreprocessedCStr, 1);

		if (!vertexShader.parse(&Resources, 100, false, messages))
		{
			KR_CORE_ERROR("{0}", vertexShader.getInfoLog());
			KR_CORE_ERROR("{0}", vertexShader.getInfoDebugLog());
			KR_CORE_ASSERT(false, "GLSL parsing failed!");
		}

		glslang::TProgram Program;
		Program.addShader(&vertexShader);

		if (!Program.link(messages))
		{
			KR_CORE_ERROR("{0}", vertexShader.getInfoLog());
			KR_CORE_ERROR("{0}", vertexShader.getInfoDebugLog());
			KR_CORE_ASSERT(false, "Shader link faliure!");
		}

		std::vector<uint32_t> SpirV;
		spv::SpvBuildLogger Logger;
		glslang::SpvOptions SpvOptions;
		glslang::GlslangToSpv(*Program.getIntermediate(EShLangVertex), SpirV, &Logger, &SpvOptions);

	}

	VulkanShader::~VulkanShader()
	{
	
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