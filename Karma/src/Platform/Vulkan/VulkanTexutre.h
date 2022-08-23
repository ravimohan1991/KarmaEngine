#pragma once

#include "Karma/Renderer/Texture.h"
#include "VulkanBuffer.h"

namespace Karma
{
	class VulkanTexture : public Texture
	{
	public:
		VulkanTexture();
		~VulkanTexture();
		
		void CreateTextureImage(VulkanImageBuffer* vImageBuffer);
		void CreateTextureImageView();
		void CreateTextureSampler();
		
		// Upload the VulkanImageBuffer to GPU when Texture is instantiated
		void GenerateVulkanTexture(VulkanImageBuffer* vImageBuffer);
		
		// Getters
		VkImageView GetImageView() const { return m_TextureImageView; }
		VkSampler GetImageSampler() const { return m_TextureSampler; }
		
	private:
		// Vulkan context relevant stuff
		VkDevice m_Device;
		VkPhysicalDevice m_PhysicalDevice;
		
		// Texture relevant stuff
		VkImage m_TextureImage;
		
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
		VkSampler m_TextureSampler;
	};
}
