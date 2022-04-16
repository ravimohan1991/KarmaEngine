#pragma once

#include "Karma/Core.h"

namespace Karma
{
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
		Texture(TextureType tType, const char* filename, std::string textureName, std::string textureShaderName);

		// Getters
		const std::string& GetTextureName() const { return m_TName; }
		const std::string& GetTextureShaderName() const { return m_TShaderName; }

	private:
		TextureType m_TType;
		std::string m_TName;
		// Name to be used for identification in the shaders
		std::string m_TShaderName;
	};
}