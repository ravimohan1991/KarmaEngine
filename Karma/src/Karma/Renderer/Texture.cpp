#include "Texture.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"
#include "Platform/Vulkan/VulkanTexutre.h"

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
				switch (Renderer::GetAPI())
				{
					case RendererAPI::API::None:
						KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
						break;
					case RendererAPI::API::OpenGL:
						ImageBuffer::Create(filename);
						break;
					case RendererAPI::API::Vulkan:
						VulkanImageBuffer* vImageBuffer = static_cast<VulkanImageBuffer*>(ImageBuffer::Create(filename));
						if(vImageBuffer != nullptr)
						{
							// Instantiate Vulkan texture rather
							// VulkanTexture::GenerateVulkanTexture(vImageBuffer);
							m_VulkanTexture = new VulkanTexture();
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
		// Experimental. Need to track the lifetime.
		delete m_VulkanTexture;
	}
}
