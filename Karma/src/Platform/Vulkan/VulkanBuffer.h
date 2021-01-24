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

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlagBits properties);

		inline VkBuffer GetVertexBuffer() const { return m_VertexBuffer; }
		inline VkDeviceMemory GetVertexBufferMemory() const { return m_VertexBufferMemory; }

	private:
		BufferLayout m_Layout;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		VkDevice m_Device;
	};

	class KARMA_API VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}