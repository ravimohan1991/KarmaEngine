#include "VulkanBuffer.h"
#include "Platform/Vulkan/VulkanHolder.h"

namespace Karma
{
	// Vertex Buffer
	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
	{
		m_Device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();
		
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_VertexBuffer);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create vertex buffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, m_VertexBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

		VkResult resultm = vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_VertexBufferMemory);

		KR_CORE_ASSERT(resultm == VK_SUCCESS, "Failed to allocate vertexbuffer memory");
		vkBindBufferMemory(m_Device, m_VertexBuffer, m_VertexBufferMemory, 0);

		// Move this to Bind()?
		void* data;
		vkMapMemory(m_Device, m_VertexBufferMemory, 0, bufferInfo.size, 0, &data);

		memcpy(data, vertices, (size_t)bufferInfo.size);
		vkUnmapMemory(m_Device, m_VertexBufferMemory);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
	}

	uint32_t VulkanVertexBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlagBits properties)
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

		KR_CORE_ASSERT(false, "Failed to find suitable memory type");
		return 0;
	}

	void VulkanVertexBuffer::Bind() const
	{

	}

	void VulkanVertexBuffer::UnBind() const
	{
	}


	// Index buffer
	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t size)
	{
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
	}

	void VulkanIndexBuffer::Bind() const
	{
	}

	void VulkanIndexBuffer::UnBind() const
	{
	}
}