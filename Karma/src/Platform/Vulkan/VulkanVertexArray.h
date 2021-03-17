#pragma once

#include "Karma/Renderer/VertexArray.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Karma/Renderer/Shader.h"
#include "Platform/Vulkan/VulkanShader.h"

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

		virtual void SetShader(std::shared_ptr<Shader> shader) override;

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
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);

		// Getters
		inline const std::vector<VkSemaphore>& GetImageAvailableSemaphore() const { return m_imageAvailableSemaphores; }
		inline const std::vector<VkSemaphore>& GetRenderFinishedSemaphore() const { return m_renderFinishedSemaphores; }
		inline std::vector<VkFence>& GetInFlightFence() { return m_inFlightFences; };
		inline std::vector<VkFence>& GetImagesInFlight() { return m_imagesInFlight; }
		const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return m_commandBuffers; }
		const int GetMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

	private:
		std::shared_ptr<VulkanShader> m_Shader;

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<VulkanVertexBuffer> m_VertexBuffer;
		std::shared_ptr<VulkanIndexBuffer> m_IndexBuffer;

		VkDevice m_device;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_graphicsPipeline;

		std::vector<VkCommandBuffer> m_commandBuffers;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;

		VkVertexInputBindingDescription m_bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

		const int MAX_FRAMES_IN_FLIGHT = 2;
	};

}