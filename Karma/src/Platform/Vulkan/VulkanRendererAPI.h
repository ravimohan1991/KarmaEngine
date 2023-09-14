#pragma once

#include "krpch.h"

#include "Karma/Renderer/RendererAPI.h"
#include "vulkan/vulkan.h"

namespace Karma
{
	class VulkanVertexArray;
	class KARMA_API VulkanRendererAPI : public RendererAPI
	{
	public:
		VulkanRendererAPI();
		virtual ~VulkanRendererAPI();
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void BeginScene() override;
		virtual void DrawIndexed(std::shared_ptr<VertexArray> vertexArray) override;
		virtual void EndScene() override;

		void AllocateCommandBuffers();
		void RecordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void SubmitCommandBuffers();
		void CreateSynchronicity();
		void ClearVulkanRendererAPI();
		void RemoveSynchronicity();
		void RecreateCommandBuffersPipelineSwapchain();
		void RecreateCommandBuffersAndSwapChain();

		// Getters. Depending on detailed implementation of other API (such as OpenGL), we may promote the getter to abstract
		const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return m_commandBuffers; }
		const int& GetMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
		const std::vector<VkFence>& GetFences() const { return m_InFlightFences; }
		const std::vector<VkSemaphore>& GetImageAvailableSemaphores() const { return m_ImageAvailableSemaphores; }
		const std::vector<VkSemaphore> GetRenderFinishedSemaphore() const { return m_RenderFinishedSemaphores; }

	private:
		size_t m_CurrentFrame = 0;

		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<std::shared_ptr<VulkanVertexArray>> m_VulkaVertexArrays;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		// Number of images (to work upon (CPU side) whilst an image is being rendered (GPU side processing)) + 1
		// Clearly, MAX_FRAMES_IN_FLIGHT shouldn't exceed m_SwapChainImages.size()
		const int MAX_FRAMES_IN_FLIGHT = 2;

		bool m_bAllocateCommandBuffers;
	};
}
