/**
 * @file VulkanTexture.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains VulkanTexture class which contains Vulkan specific implementation of Texture class.
 * @version 1.0
 * @date July 4, 2021
 * 
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/Texture.h"
#include "VulkanBuffer.h"

namespace Karma
{
	class FVulkanDevice;

	/**
	 * @brief Vulkan specific implementation of Texture class.
	 * 
	 * This class handles the creation and management of Vulkan textures, including image views and samplers.
	 * 
	 * A sampler in Vulkan is an object that defines how to sample textures in shaders. It encapsulates sampling 
	 * parameters such as filtering modes, addressing modes, and mipmapping settings.
	 * 
	 * @since Karma 1.0.0
	 */
	class VulkanTexture : public Texture
	{
	public:
		/**
		 * @brief A constructor
		 * 
		 * @since Karma 1.0.0
		 */
		VulkanTexture();

		/**
		 * @brief Constructor with VulkanRHI support
		 * 
		 * @brief InDevice						The FVulkanDevice object
		 * @brief filename						The location of the texture file along with location
		 * 
		 * @see FVulkanDevice::InitGPU()
		 * @since Karma 1.0.0
		 */
		VulkanTexture(FVulkanDevice* InDevice, const char* filename);

		/**
		 * @brief A destructor
		 * 
		 * Cleans up vulkan resources associated with the texture which includes destroying the image sampler, 
		 * image view, image, and freeing the image memory.
		 * 
		 * @since Karma 1.0.0
		 */
		~VulkanTexture();
		
		/**
		 * @brief Creates Vulkan image for the texture and allocates device memory appropriately
		 * 
		 * There are also transitions of (m_TextureImage) image layouts (from VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		 * then copying of vImageBuffer to m_TextureImage followed by transition VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		 * and vkBindImageMemory call to bind the allocated memory to the image.
		 * 
		 * Layout transitions are done because different operations in Vulkan require images to be in specific layouts for optimal performance and correctness.
		 * https://vulkan-tutorial.com/Texture_mapping/Images
		 * 
		 * VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Optimal as destination in a transfer operation, like vkCmdCopyBufferToImage
		 * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: Optimal for reading in shaders, used when sampling the texture in a shader.
		 * 
		 * @param vImageBuffer					The VulkanImageBuffer containing the image data to be used for the texture
		 *										VulkanImageBuffer is expected to have been created and populated with image data prior to this call.
		 *										See ImageBuffer::Create()
		 * @see VulkanTexture::GenerateVulkanTexture(), Texture::Texture()
		 */
		void CreateTextureImage(VulkanImageBuffer* vImageBuffer);

		/**
		 * @brief Creates the image view for the texture
		 * 
		 * @since Karma 1.0.0
		 */
		void CreateTextureImageView();

		/**
		 * @brief Creates the texture sampler
		 * 
		 * A sampler in Vulkan defines how to sample textures in shaders, including filtering modes, addressing modes, and mipmapping settings.
		 * 
		 * @since Karma 1.0.0
		 */
		void CreateTextureSampler();
		
		/**
		 * @brief Generates the Vulkan texture by creating the image, image view, and sampler
		 * 
		 * This function orchestrates the creation of the Vulkan texture by calling CreateTextureImage, 
		 * CreateTextureImageView, and CreateTextureSampler in sequence.
		 * 
		 * @param vImageBuffer					The VulkanImageBuffer containing the image data to be used for the texture
		 * 
		 * @see VulkanTexture::CreateTextureImage(), VulkanTexture::CreateTextureImageView(), VulkanTexture::CreateTextureSampler()
		 * @since Karma 1.0.0
		 */
		void GenerateVulkanTexture(VulkanImageBuffer* vImageBuffer);

		/**
		 * @brief Finds a suitable memory type on the physical device (graphics card) based on the type filter and desired properties.
		 *
		 * @param typeFilter						Bitmask specifying the acceptable memory types
		 * @param properties						Desired memory properties (like VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT for CPU access)
		 *
		 * @see VulkanBuffer::VulkanBuffer()
		 * @since Karma 1.0.0
		 */
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		
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
