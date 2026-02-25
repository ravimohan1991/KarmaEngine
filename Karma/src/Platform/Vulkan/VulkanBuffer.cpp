#include "VulkanBuffer.h"
#include "Karma/KarmaUtilities.h"
#include "VulkanRHI/VulkanDynamicRHI.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanSwapChain.h"
#include "KarmaGui/KarmaGuiRenderer.h"
#include "VulkanRHI/VulkanDescriptorSets.h"

namespace Karma
{
	// Vertex Buffer
	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
	{
		m_Device = FVulkanDynamicRHI::Get().GetDevice()->GetLogicalDevice();

		VkDeviceSize bufferSize = size;
		m_BufferSize = size;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, vertices, (size_t)size);
		vkUnmapMemory(m_Device, stagingBufferMemory);

		CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VertexBuffer, m_VertexBufferMemory);

		CopyBuffer(stagingBuffer, m_VertexBuffer, size);

		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		vkDeviceWaitIdle(m_Device);

		vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
	}

	void VulkanVertexBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = FVulkanDynamicRHI::Get().GetDevice()->GetCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(FVulkanDynamicRHI::Get().GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(FVulkanDynamicRHI::Get().GetDevice()->GetGraphicsQueue());

		vkFreeCommandBuffers(m_Device, allocInfo.commandPool, 1, &commandBuffer);
	}

	void VulkanVertexBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create vertexbuffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FVulkanDynamicRHI::Get().FindMemoryType(memRequirements.memoryTypeBits, properties);

		VkResult resultm = vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory);

		KR_CORE_ASSERT(resultm == VK_SUCCESS, "Failed to allocate vertexbuffer memory");
		vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
	}

	void VulkanVertexBuffer::Bind() const
	{

	}

	void VulkanVertexBuffer::UnBind() const
	{
	}

	// Index buffer
	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count) : m_Count(count)
	{
		m_Device = FVulkanDynamicRHI::Get().GetDevice()->GetLogicalDevice();

		VkDeviceSize bufferSize = sizeof(uint32_t) * count;
		m_BufferSize = bufferSize;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices, (size_t)bufferSize);
		vkUnmapMemory(m_Device, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_IndexBuffer, m_IndexBufferMemory);

		CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		vkDeviceWaitIdle(m_Device);

		vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
		vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);
	}

	void VulkanIndexBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = FVulkanDynamicRHI::Get().GetDevice()->GetCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(FVulkanDynamicRHI::Get().GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(FVulkanDynamicRHI::Get().GetDevice()->GetGraphicsQueue());

		vkFreeCommandBuffers(m_Device, allocInfo.commandPool, 1, &commandBuffer);
	}

	void VulkanIndexBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create indexbuffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FVulkanDynamicRHI::Get().FindMemoryType(memRequirements.memoryTypeBits, properties);

		VkResult resultm = vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory);

		KR_CORE_ASSERT(resultm == VK_SUCCESS, "Failed to allocate indexbuffer memory");
		vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
	}

	void VulkanIndexBuffer::Bind() const
	{
	}

	void VulkanIndexBuffer::UnBind() const
	{
	}

	// Uniform buffer
	VulkanUniformBuffer::VulkanUniformBuffer(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex) :
		UniformBufferObject(dataTypes, bindingPointIndex)
	{
		m_Device = FVulkanDynamicRHI::Get().GetDevice()->GetLogicalDevice();
		BufferCreation();

		//std::shared_ptr <VulkanUniformBuffer> uboPtr(this);
		//VulkanHolder::GetVulkanContext()->RegisterUBO(this);
		
		FVulkanDynamicRHI::Get().RegisterUniformBufferObject(this);
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		ClearBuffer();
	}

	void VulkanUniformBuffer::UpdateCameraUniform()
	{
		uint32_t maxFramesInFlight = KarmaGuiRenderer::GetWindowData().RHIResources->VulkanSwapChain->GetMaxFramesInFlight();
		
		for(uint32_t counter = 0; counter < maxFramesInFlight; counter++)
		{
			FVulkanDynamicRHI::Get().GetDevice()->GetDefaultDescriptorSets()[counter]->UpdateUniformBufferDescriptorSet(this, 0, 0, counter);
		}
	}

	void VulkanUniformBuffer::BufferCreation()
	{
		VkDeviceSize bufferSize = GetBufferSize();
		
		int maxFramesInFlight = KarmaGuiRenderer::GetWindowData().RHIResources->VulkanSwapChain->GetMaxFramesInFlight();

		m_UniformBuffers.resize(maxFramesInFlight);
		m_UniformBuffersMemory.resize(maxFramesInFlight);

		for (size_t i = 0; i < maxFramesInFlight; i++)
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);
		}
	}

	void VulkanUniformBuffer::ClearBuffer()
	{
		vkDeviceWaitIdle(m_Device);

		for (size_t i = 0; i < m_UniformBuffers.size(); i++)
		{
			vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
			vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
		}
	}

	void VulkanUniformBuffer::UploadUniformBuffer(size_t frameIndex)
	{
		uint32_t index = 0;
		for (auto& it : GetUniformList())
		{
			size_t uniformSize = GetUniformSize()[index];
			size_t offset = GetAlignedOffsets()[index++];
			void* data;
			vkMapMemory(m_Device, m_UniformBuffersMemory[frameIndex], offset, uniformSize, 0, &data);
			memcpy(data, it.GetDataPointer(), uniformSize);
			vkUnmapMemory(m_Device, m_UniformBuffersMemory[frameIndex]);
		}
	}

	void VulkanUniformBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create uniformbuffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FVulkanDynamicRHI::Get().FindMemoryType(memRequirements.memoryTypeBits, properties);

		VkResult resultm = vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory);

		KR_CORE_ASSERT(resultm == VK_SUCCESS, "Failed to allocate uniformbuffer memory");
		vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
	}

	// ImageBuffer
	VulkanImageBuffer::VulkanImageBuffer(const char* filename)
	{
		stbi_uc* pixels = KarmaUtilities::GetImagePixelData(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		// Need more consideration on image size
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		KR_CORE_ASSERT(pixels, "Failed to load textures image!");

		m_Device = FVulkanDynamicRHI::Get().GetDevice()->GetLogicalDevice();
		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_StagingBuffer, m_StagingBufferMemory);
		void* data;
		vkMapMemory(m_Device, m_StagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_Device, m_StagingBufferMemory);

		stbi_image_free(pixels);
	}

	VulkanImageBuffer::VulkanImageBuffer(FVulkanDevice* InDevice, const char* filename)
	{
		stbi_uc* pixels = KarmaUtilities::GetImagePixelData(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		// Need more consideration on image size
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		KR_CORE_ASSERT(pixels, "Failed to load textures image!");

		m_Device = InDevice->GetLogicalDevice();
		m_PhysicalDevice = InDevice->GetGPU();

		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_StagingBuffer, m_StagingBufferMemory);
		void* data;
		vkMapMemory(m_Device, m_StagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_Device, m_StagingBufferMemory);

		stbi_image_free(pixels);
	}

	VulkanImageBuffer::~VulkanImageBuffer()
	{
		vkDestroyBuffer(m_Device, m_StagingBuffer, nullptr);
		vkFreeMemory(m_Device, m_StagingBufferMemory, nullptr);
	}

	void VulkanImageBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create uniformbuffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FVulkanDynamicRHI::Get().FindMemoryType(memRequirements.memoryTypeBits, properties);

		VkResult resultm = vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory);

		KR_CORE_ASSERT(resultm == VK_SUCCESS, "Failed to allocate imagebuffer memory");
		vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
	}
}
