#pragma once

#include "krpch.h"

namespace Karma
{
class VulkanTexture;

	enum class TextureType
	{
		None = 0,
		Image,
		LightMap,
		DiffusionMap
	};

	class KARMA_API Texture
	{
	public:
		Texture();
		Texture(TextureType tType, const char* filename, std::string textureName, std::string textureShaderName);
		
		~Texture();

		// Getters
		const std::string& GetTextureName() const { return m_TName; }
		const std::string& GetTextureShaderName() const { return m_TShaderName; }
		VulkanTexture* GetVulkanTexture() const { return m_VulkanTexture; }

	private:
		TextureType m_TType;
		std::string m_TName;
		// Name to be used for identification in the shaders
		std::string m_TShaderName;

		// For Vulkan specific purposes
		VulkanTexture* m_VulkanTexture;
	};
}
