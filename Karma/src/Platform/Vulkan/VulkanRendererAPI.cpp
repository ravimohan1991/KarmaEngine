#include "VulkanRendererAPI.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanHolder.h"
#include "Platform/Vulkan/VulkanVertexArray.h"

namespace Karma
{
	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
	}
	
	void VulkanRendererAPI::Clear()
	{
	}

	void VulkanRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		std::shared_ptr<VulkanVertexArray> vulkanVA = std::static_pointer_cast<VulkanVertexArray>(vertexArray);

		vkWaitForFences(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), 1, &vulkanVA->GetInFlightFence()[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult resultAI = vkAcquireNextImageKHR(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), VulkanHolder::GetVulkanContext()->GetSwapChain(), UINT64_MAX, vulkanVA->GetImageAvailableSemaphore()[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (resultAI == VK_ERROR_OUT_OF_DATE_KHR)
		{
			vulkanVA->RecreateSwapChainAndPipeline();
		}
		else if (resultAI != VK_SUCCESS && resultAI != VK_SUBOPTIMAL_KHR)
		{
			KR_CORE_ASSERT(false, "Failed to acquire swapchain image");
		}

		if (vulkanVA->GetImagesInFlight()[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), 1, &vulkanVA->GetImagesInFlight()[imageIndex], VK_TRUE, UINT64_MAX);
		}

		vulkanVA->GetImagesInFlight()[imageIndex] = vulkanVA->GetInFlightFence()[m_CurrentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { vulkanVA->GetImageAvailableSemaphore()[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(vulkanVA->GetCommandBuffers()[imageIndex]);

		VkSemaphore signalSemaphores[] = { vulkanVA->GetRenderFinishedSemaphore()[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), 1, &vulkanVA->GetInFlightFence()[m_CurrentFrame]);
		
		VkResult result = vkQueueSubmit(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), 1, &submitInfo, vulkanVA->GetInFlightFence()[m_CurrentFrame]);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit draw command buffer");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { VulkanHolder::GetVulkanContext()->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		VkResult resultQP = vkQueuePresentKHR(VulkanHolder::GetVulkanContext()->GetPresentQueue(), &presentInfo);

		if (resultQP == VK_ERROR_OUT_OF_DATE_KHR || resultQP == VK_SUBOPTIMAL_KHR)
		{
			vulkanVA->RecreateSwapChainAndPipeline();
		}
		else if (resultQP != VK_SUCCESS)
		{
			KR_CORE_ASSERT(false, "Failed to present swapchain image");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % vulkanVA->GetMaxFramesInFlight();
	}
}