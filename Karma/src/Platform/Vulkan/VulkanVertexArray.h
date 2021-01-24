#pragma once

#include "Karma/Renderer/VertexArray.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Karma
{
	class KARMA_API VulkanVertexArray : public VertexArray
	{
	public:
		VulkanVertexArray();
		virtual ~VulkanVertexArray();

		virtual void Bind() const override;
		virtual void UnBind() const override {}

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override {}

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPools();
		void CreateCommandBuffers();

		void GenerateVulkanVA();

		void CreateSemaphores();

		// Helper functions
		static std::vector<char> ReadFile(const std::string& filename);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

	private:
		uint32_t m_RendererID;

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<VulkanVertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		VkDevice m_device;
		VkExtent2D m_swapChainExtent;
		VkRenderPass m_renderPass;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_graphicsPipeline;

		std::vector<VkFramebuffer> m_swapChainFrameBuffers;

		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers;

		VkSemaphore m_imageAvailableSemaphore;
		VkSemaphore m_renderFinishedSemaphore;

		VkVertexInputBindingDescription m_bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

		//VkBuffer m_vertexBuffer;
		//VkDeviceMemory m_vertexBufferMemory;
	};

}