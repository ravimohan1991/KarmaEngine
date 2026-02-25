#include "Texture.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "KarmaRHI/DynamicRHI.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Karma
{
	Texture::Texture()
	{
	}

	Texture::Texture(TextureType tType, const char* filename, std::string textureName, std::string textureShaderName) : m_TType(tType),
		m_TName(textureName), m_TShaderName(textureShaderName)
	{
		switch (tType)
		{
		case TextureType::Image:
		{
            switch (GRHIInterfaceType)
			{
            case ERHIInterfaceType::Null:
                KR_CORE_ASSERT(false, "Without graphics API not supported");
				break;
            case ERHIInterfaceType::OpenGL:
				ImageBuffer::Create(filename);
				break;
            case ERHIInterfaceType::Vulkan:
				VulkanImageBuffer* vImageBuffer = static_cast<VulkanImageBuffer*>(ImageBuffer::Create(filename));
				if (vImageBuffer != nullptr)
				{
					// Instantiate Vulkan texture rather
					// VulkanTexture::GenerateVulkanTexture(vImageBuffer);
					m_VulkanTexture.reset(new VulkanTexture());
					m_VulkanTexture->GenerateVulkanTexture(vImageBuffer);
				}
				delete vImageBuffer;
				break;
			}
			break;
		}
		case TextureType::DiffusionMap:
		case TextureType::LightMap:

		case TextureType::None:
			KR_CORE_ASSERT(false, "Other maps not supported yet.");
			break;
		}
	}

	Texture::~Texture()
	{
	}
}
