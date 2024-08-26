#include "Karma/Renderer/Material.h"
#include "glm/glm.hpp"

namespace Karma
{
	Material::Material() : m_MainCamera(nullptr), m_ModelTranslation(glm::vec3(0.f, 0.f, 0.f)), m_ModelRotation(glm::vec3(0.f, 0.f, 0.f)),
    m_ModelScale(glm::vec3(1.f, 1.f, 1.f))
	{
	}

	void Material::OnUpdate()
	{
		if (m_MainCamera != nullptr)
		{
			m_MaterialProjectionMatrix = m_MainCamera->GetProjectionMatrix();
			m_MaterialViewMatrix       = glm::scale(m_MainCamera->GetViewMatirx() * GetModelTransformMatrix(), m_ModelScale);
			
			// Apply GuizmoTransform
			
			UBODataPointer uProjection(&m_MaterialProjectionMatrix);
			UBODataPointer uView(&m_MaterialViewMatrix);

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
        
        m_MaterialProjectionMatrix = m_MainCamera->GetProjectionMatrix();
        m_MaterialViewMatrix       = m_MainCamera->GetViewMatirx(); 
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

	// Need to fine tune the getters by index.
	// We are with the assumption that there be only single shader and texture.
	std::shared_ptr<Shader> Material::GetShader(int index)
	{
		for (const auto& elem : m_Shaders)
		{
				return elem;
		}
		KR_CORE_WARN("Couldn't find the Shader with index {0}", index);
		return nullptr;
	}

	std::shared_ptr<Texture> Material::GetTexture(int index)
	{
		for (const auto& elem : m_Textures)
		{
				return elem;
		}
		KR_CORE_WARN("Couldn't find the Texture with index {0}", index);
		return nullptr;
	}
}
