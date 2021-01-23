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

		inline float* GetData() const { return m_Vertices; }

	private:
		float* m_Vertices;
		BufferLayout m_Layout;
		
		VkDevice m_Device;
		VkPhysicalDevice m_PhysicalDevice;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;
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