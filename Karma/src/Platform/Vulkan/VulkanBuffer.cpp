#include "VulkanBuffer.h"
#include "Platform/Vulkan/VulkanHolder.h"

namespace Karma
{
	// Vertex Buffer
	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
	{
		m_Device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();

		VkDeviceSize bufferSize = size;

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
		vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
	}

	void VulkanVertexBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VulkanHolder::GetVulkanContext()->GetCommandPool();
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

		vkQueueSubmit(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(VulkanHolder::GetVulkanContext()->GetGraphicsQueue());

		vkFreeCommandBuffers(m_Device, VulkanHolder::GetVulkanContext()->GetCommandPool(), 1, &commandBuffer);
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
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		VkResult resultm = vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory);

		KR_CORE_ASSERT(resultm == VK_SUCCESS, "Failed to allocate vertexbuffer memory");
		vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
	}

	uint32_t VulkanVertexBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanHolder::GetVulkanContext()->GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		KR_CORE_ASSERT(false, "Failed to find suitable memory type for vertexbuffer");
		return 0;
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
		m_Device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();

		VkDeviceSize bufferSize = sizeof(uint32_t) * count;

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
		vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
		vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);
	}

	void VulkanIndexBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VulkanHolder::GetVulkanContext()->GetCommandPool();
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

		vkQueueSubmit(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(VulkanHolder::GetVulkanContext()->GetGraphicsQueue());

		vkFreeCommandBuffers(m_Device, VulkanHolder::GetVulkanContext()->GetCommandPool(), 1, &commandBuffer);
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
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		VkResult resultm = vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory);

		KR_CORE_ASSERT(resultm == VK_SUCCESS, "Failed to allocate indexbuffer memory");
		vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
	}

	uint32_t VulkanIndexBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanHolder::GetVulkanContext()->GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		KR_CORE_ASSERT(false, "Failed to find suitable memory type for indexbuffer");
		return 0;
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
		m_Device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();
		BufferCreation();
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		//ClearBuffer();
	}

	void VulkanUniformBuffer::BufferCreation()
	{
		VkDeviceSize bufferSize = GetBufferSize();

		size_t swapChainImagesSize = VulkanHolder::GetVulkanContext()->GetSwapChainImages().size();

		m_UniformBuffers.resize(swapChainImagesSize);
		m_UniformBuffersMemory.resize(swapChainImagesSize);

		for (size_t i = 0; i < swapChainImagesSize; i++)
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);
		}
	}

	void VulkanUniformBuffer::ClearBuffer()
	{
		for (size_t i = 0; i < m_UniformBuffers.size(); i++)
		{
			vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
			vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
		}
	}

	void VulkanUniformBuffer::UploadUniformBuffer(size_t currentImage)
	{
		uint32_t index = 0;
		for (auto& it : GetUniformList())
		{
			size_t uniformSize = GetUniformSize()[index];
			size_t offset = GetAlignedOffsets()[index++];
			void* data;
			vkMapMemory(m_Device, m_UniformBuffersMemory[currentImage], offset, uniformSize, 0, &data);
			memcpy(data, it.GetDataPointer(), uniformSize);
			vkUnmapMemory(m_Device, m_UniformBuffersMemory[currentImage]);
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
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		VkResult resultm = vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory);

		KR_CORE_ASSERT(resultm == VK_SUCCESS, "Failed to allocate uniformbuffer memory");
		vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
	}

	uint32_t VulkanUniformBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanHolder::GetVulkanContext()->GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		KR_CORE_ASSERT(false, "Failed to find suitable memory type for uniformbuffer");
		return 0;
	}
}