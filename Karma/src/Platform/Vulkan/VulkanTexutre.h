#pragma once

#include "Karma/Renderer/Texture.h"
#include "VulkanBuffer.h"

namespace Karma
{
	class VulkanTexture : public Texture
	{
	public:
		// Upload the VulkanImageBuffer to GPU when Texture is instantiated
		static void GenerateVulkanTexture(VulkanImageBuffer* vImageBuffer);
	};
}
