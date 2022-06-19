#include "Karma/Renderer/Material.h"

namespace Karma
{
	Material::Material() : m_MainCamera(nullptr)
	{
	}

	void Material::OnUpdate()
	{
		if (m_MainCamera != nullptr)
		{
			UBODataPointer uProjection(&m_MainCamera->GetProjectionMatrix());
			UBODataPointer uView(&m_MainCamera->GetViewMatirx());

			// Hack for now
			for (const auto& elem : m_Shaders)
			{
				elem->GetUniformBufferObject()->UpdateUniforms(uProjection, uView);
			}
		}
		else
		{
			KR_CORE_WARN("No main camera specified for Material.");
		}
	}

	void Material::AttatchMainCamera(std::shared_ptr<Camera> mCamera)
	{
		m_MainCamera = mCamera;
	}

	void Material::ProcessForSubmission()
	{
		// Hack for now. We shall systematically deal with this as complexity of Material increases

		for (const auto& elem : m_Shaders)
		{
			// Again hacky way.
			elem->Bind(m_Textures.front()->GetTextureShaderName());
			elem->GetUniformBufferObject()->UploadUniformBuffer();
		}
	}

	std::shared_ptr<Shader> Material::GetShader(const std::string& shaderName) const
	{
		for (const auto& elem : m_Shaders)
		{
			if (elem->GetShaderName() == shaderName)
			{
				return elem;
			}
		}
		KR_CORE_WARN("Couldn't find the Shader with name {0}", shaderName.c_str());
		return nullptr;
	}
}