#include "VulkanRendererAPI.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanHolder.h"
#include "Platform/Vulkan/VulkanVertexArray.h"

namespace Karma
{
	VulkanRendererAPI::VulkanRendererAPI()
	{
	}
	
	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
	}
	
	void VulkanRendererAPI::Clear()
	{
	}

	void VulkanRendererAPI::CreateSynchronicity()
	{
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(VulkanHolder::GetVulkanContext()->GetSwapChainImages().size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkDevice device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkResult resulti = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]);
			KR_CORE_ASSERT(resulti == VK_SUCCESS, "Failed to create imageAvailableSemaphore");

			VkResult resultr = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
			KR_CORE_ASSERT(resultr == VK_SUCCESS, "Failed to create renderFinishedSemaphore");

			VkResult resultf = vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]);
			KR_CORE_ASSERT(resultf == VK_SUCCESS, "Failed to create inFlightFence");
		}
	}

	void VulkanRendererAPI::RemoveSynchronicity()
	{
		VkDevice device = VulkanHolder::GetVulkanContext()->GetLogicalDevice();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, m_InFlightFences[i], nullptr);
		}
	}

	void VulkanRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		std::shared_ptr<VulkanVertexArray> vulkanVA = std::static_pointer_cast<VulkanVertexArray>(vertexArray);

		vkWaitForFences(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult resultAI = vkAcquireNextImageKHR(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), VulkanHolder::GetVulkanContext()->GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (resultAI == VK_ERROR_OUT_OF_DATE_KHR)
		{
			vulkanVA->RecreateSwapChainAndPipeline();
		}
		else if (resultAI != VK_SUCCESS && resultAI != VK_SUBOPTIMAL_KHR)
		{
			KR_CORE_ASSERT(false, "Failed to acquire swapchain image");
		}

		if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

		vulkanVA->GetShader()->GetUniformBufferObject()->UploadUniformBuffer(imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(vulkanVA->GetCommandBuffers()[imageIndex]);

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
		
		VkResult result = vkQueueSubmit(VulkanHolder::GetVulkanContext()->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
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

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
}