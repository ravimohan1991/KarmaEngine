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
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const std::shared_ptr<IndexBuffer> GetIndexBuffer() const override { return m_IndexBuffer; }

		void CreateGraphicsPipeline();

		void CreateCommandBuffers();

		void GenerateVulkanVA();

		void CreateSemaphores();

		void RecreateSwapChainAndPipeline();
		void CleanupPipelineandCommandBuffers();

		// Helper functions
		static std::vector<char> ReadFile(const std::string& filename);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		// Getters
		inline VkSemaphore GetImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
		inline VkSemaphore GetRenderFinishedSemaphore() const { return m_renderFinishedSemaphore; }
		const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return m_commandBuffers; }

	private:
		uint32_t m_RendererID;

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<VulkanVertexBuffer> m_VertexBuffer;
		std::shared_ptr<VulkanIndexBuffer> m_IndexBuffer;

		VkDevice m_device;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_graphicsPipeline;

		std::vector<VkCommandBuffer> m_commandBuffers;

		VkSemaphore m_imageAvailableSemaphore;
		VkSemaphore m_renderFinishedSemaphore;

		VkVertexInputBindingDescription m_bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
	};

}