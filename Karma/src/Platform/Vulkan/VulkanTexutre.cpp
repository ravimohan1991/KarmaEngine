#include "VulkanTexutre.h"
#include "VulkanHolder.h"

namespace Karma
{
	void VulkanTexture::GenerateVulkanTexture(VulkanImageBuffer* vImageBuffer)
	{
		auto vContext = VulkanHolder::GetVulkanContext();
		vContext->CreateTextureImage(vImageBuffer);
		vContext->CreateTextureImageView();
		vContext->CreateTextureSampler();
	}
}