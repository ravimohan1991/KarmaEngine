#pragma once

#include "Karma/Renderer/Buffer.h"
#include "vulkan/vulkan.h"

namespace Karma
{
	class KARMA_API VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(float* vertices, uint32_t size);
		virtual ~VulkanVertexBuffer();

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual const BufferLayout& GetLayout() const override
		{
			return m_Layout;
		}
		virtual void SetLayout(const BufferLayout& layout) override
		{
			m_Layout = layout;
		}

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		inline VkBuffer GetVertexBuffer() const { return m_VertexBuffer; }
		inline VkDeviceMemory GetVertexBufferMemory() const { return m_VertexBufferMemory; }

		inline size_t GetBufferSize() { return m_BufferSize; }

	private:
		VkDevice m_Device;
		BufferLayout m_Layout;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		size_t m_BufferSize;
	};

	class KARMA_API VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();

		virtual void Bind() const override;
		virtual void UnBind() const override;

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		virtual uint32_t GetCount() const override { return m_Count; }
		inline VkBuffer GetIndexBuffer() const { return m_IndexBuffer; }
		inline VkDeviceMemory GetIndexBufferMemory() const { return m_IndexBufferMemory; }

		inline size_t GetBufferSize() { return m_BufferSize; }

	private:
		VkDevice m_Device;
		uint32_t m_Count;

		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		size_t m_BufferSize;
	};

	struct KARMA_API VulkanUniformBuffer : public UniformBufferObject
	{
	public:
		VulkanUniformBuffer(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex);
		virtual ~VulkanUniformBuffer();

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		const std::vector<VkBuffer>& GetUniformBuffers() const { return m_UniformBuffers; }

		void ClearBuffer();
		void BufferCreation();

		void UploadUniformBuffer(size_t frameIndex);

	private:
		VkDevice m_Device;
		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
	};

	class KARMA_API VulkanImageBuffer : public ImageBuffer
	{
	public:
		VulkanImageBuffer(const char* filename);
		virtual ~VulkanImageBuffer();
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		const inline VkBuffer& GetBuffer() const { return m_StagingBuffer; }

		// Getters
		int GetTextureWidth() const { return texWidth; }
		int GetTextureHeight() const { return texHeight; }
		int GetTextureChannels() const { return texChannels; }

	private:
		VkDevice m_Device;
		VkBuffer m_StagingBuffer;
		VkDeviceMemory m_StagingBufferMemory;

		// Image props (properties)
		int texWidth;
		int texHeight;
		int texChannels;
	};
}
