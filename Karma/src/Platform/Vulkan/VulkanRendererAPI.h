#pragma once

#include "Karma/Renderer/RendererAPI.h"
#include "Karma/Core.h"
#include "vulkan/vulkan.h"

namespace Karma
{
	class KARMA_API VulkanRendererAPI : public RendererAPI
	{
	public:
		VulkanRendererAPI();
		
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

		void CreateSynchronicity();
		void RemoveSynchronicity();

	private:
		size_t m_CurrentFrame = 0;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		const int MAX_FRAMES_IN_FLIGHT = 2;
	};
}