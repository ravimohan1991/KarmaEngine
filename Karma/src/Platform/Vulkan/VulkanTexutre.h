#pragma once

#include "Karma/Renderer/Texture.h"
#include "VulkanBuffer.h"

namespace Karma
{
	class VulkanTexture : public Texture
	{
	public:
		static void GenerateVulkanTexture(VulkanImageBuffer* vImageBuffer);
	};
}
