#pragma once

#include "Karma/Renderer/RendererAPI.h"
#include "Karma/Core.h"
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
		void RecordCommandBuffers();
		void SubmitCommandBuffers();
		void CreateSynchronicity();
		void ClearVulkanRendererAPI();
		void RemoveSynchronicity();
		void RecreateCommandBuffersPipelineSwapchain();

	private:
		size_t m_CurrentFrame = 0;
		glm::vec4 m_ClearColor;

		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<std::shared_ptr<VulkanVertexArray>> m_VulkaVertexArrays;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		const int MAX_FRAMES_IN_FLIGHT = 2;
	};
}