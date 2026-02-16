#include "VulkanTexture.h"
#include "VulkanHolder.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanDynamicRHI.h"

namespace Karma
{
	VulkanTexture::VulkanTexture()
	{
		m_LogicalDevice = VulkanHolder::GetVulkanContext()->GetLogicalDevice();
		m_GPU = VulkanHolder::GetVulkanContext()->GetPhysicalDevice();
	}

	VulkanTexture::VulkanTexture(FVulkanDevice* InDevice, const char* filename) : m_TextureImage(VK_NULL_HANDLE), 
		m_TextureImageMemory(VK_NULL_HANDLE), m_TextureImageView(VK_NULL_HANDLE), m_TextureSampler(VK_NULL_HANDLE)
	{
		m_Device = InDevice;

		m_LogicalDevice = InDevice->GetLogicalDevice();
		m_GPU = InDevice->GetGPU();

		VulkanImageBuffer* vImageBuffer = new VulkanImageBuffer(InDevice, filename);
		if (vImageBuffer != nullptr)
		{
			GenerateVulkanTexture(vImageBuffer);
		}
		else
		{
			KR_CORE_WARN("Couldn't load the image texture file {0}", filename);
		}
		delete vImageBuffer;
	}

	VulkanTexture::~VulkanTexture()
	{
		vkDestroySampler(m_LogicalDevice, m_TextureSampler, nullptr);
		vkDestroyImageView(m_LogicalDevice, m_TextureImageView, nullptr);
		vkDestroyImage(m_LogicalDevice, m_TextureImage, nullptr);
		vkFreeMemory(m_LogicalDevice, m_TextureImageMemory, nullptr);
	}

	void VulkanTexture::GenerateVulkanTexture(VulkanImageBuffer* vImageBuffer)
	{
		CreateTextureImage(vImageBuffer);
		CreateTextureImageView();
		CreateTextureSampler();
	}

	void VulkanTexture::CreateTextureImage(VulkanImageBuffer* vImageBuffer)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(vImageBuffer->GetTextureWidth());
		imageInfo.extent.height = static_cast<uint32_t>(vImageBuffer->GetTextureHeight());
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VkResult result = vkCreateImage(m_LogicalDevice, &imageInfo, nullptr, &m_TextureImage);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create image!");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_LogicalDevice, m_TextureImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FVulkanDynamicRHI::Get().FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkResult result1 = vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &m_TextureImageMemory);
		KR_CORE_ASSERT(result1 == VK_SUCCESS, "Failed to allocate image memeory");

		vkBindImageMemory(m_LogicalDevice, m_TextureImage, m_TextureImageMemory, 0);

		m_Device->TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_Device->CopyBufferToImage(vImageBuffer->GetBuffer(), m_TextureImage, static_cast<uint32_t>(vImageBuffer->GetTextureWidth()), static_cast<uint32_t>(vImageBuffer->GetTextureHeight()));
		m_Device->TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanTexture::CreateTextureImageView()
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_TextureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(m_LogicalDevice, &viewInfo, nullptr, &m_TextureImageView);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create texture image view");
	}

	void VulkanTexture::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_GPU, &properties);

		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkResult result = vkCreateSampler(m_LogicalDevice, &samplerInfo, nullptr, &m_TextureSampler);

		KR_CORE_ASSERT(result == false, "Failed to create texture sampler!");
	}
}
