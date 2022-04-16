#include "Texture.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"

namespace Karma
{
	Texture::Texture(TextureType tType, const char* filename, std::string textureName, std::string textureShaderName) : m_TType(tType), 
		m_TName(textureName), m_TShaderName(textureShaderName)
	{
		switch (tType)
		{
			case TextureType::Image:
			{
				ImageBuffer::Create(filename);
				break;
			}
			case TextureType::DiffusionMap:
			case TextureType::LightMap:
			case TextureType::None:
				KR_CORE_ASSERT(false, "Other maps not supported yet.");
				break;
		}			
	}
}