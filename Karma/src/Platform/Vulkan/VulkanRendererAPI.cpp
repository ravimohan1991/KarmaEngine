#include "VulkanRendererAPI.h"
#include "vulkan/vulkan.h"
#include "Platform/Vulkan/VulkanHolder.h"
#include "Platform/Vulkan/VulkanVertexArray.h"

namespace Karma
{
	VulkanRendererAPI::VulkanRendererAPI() : m_bAllocateCommandBuffers(true)
	{
	}

	VulkanRendererAPI::~VulkanRendererAPI()
	{

	}

	void VulkanRendererAPI::ClearVulkanRendererAPI()
	{
		vkDeviceWaitIdle(VulkanHolder::GetVulkanContext()->GetLogicalDevice());

		RemoveSynchronicity();
		if(m_commandBuffers.size() > 0)
		{
			vkFreeCommandBuffers(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), VulkanHolder::GetVulkanContext()->GetCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		}
	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
	}

	void VulkanRendererAPI::Clear()
	{
	}

	void VulkanRendererAPI::BeginScene()
	{
		if (m_bAllocateCommandBuffers)
		{
			AllocateCommandBuffers();
			m_bAllocateCommandBuffers = false;
		}
	}

	void VulkanRendererAPI::AllocateCommandBuffers()
	{
		m_commandBuffers.resize(VulkanHolder::GetVulkanContext()->GetSwapChainFrameBuffer().size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanHolder::GetVulkanContext()->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

		VkResult result = vkAllocateCommandBuffers(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), &allocInfo, m_commandBuffers.data());

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create command buffers!");
	}

	void VulkanRendererAPI::RecordCommandBuffers()
	{
		for (size_t i = 0; i < m_commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;

			VkResult result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);

			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to begin recording command buffer");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = VulkanHolder::GetVulkanContext()->GetRenderPass();
			renderPassInfo.framebuffer = VulkanHolder::GetVulkanContext()->GetSwapChainFrameBuffer()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = VulkanHolder::GetVulkanContext()->GetSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0] = { m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			for (auto vulkanVA : m_VulkaVertexArrays)
			{
				vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanVA->GetGraphicsPipeline());

				// Bind vertex/index buffers
				VkBuffer vertexBuffers[] = { vulkanVA->GetVertexBuffer()->GetVertexBuffer() };
				VkDeviceSize offsets[] = { 0 };

				vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(m_commandBuffers[i], vulkanVA->GetIndexBuffer()->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanVA->GetGraphicsPipelineLayout(), 0, 1, &vulkanVA->GetDescriptorSets()[i], 0, nullptr);

				vkCmdDrawIndexed(m_commandBuffers[i], vulkanVA->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
			}

			vkCmdEndRenderPass(m_commandBuffers[i]);

			VkResult resultCB = vkEndCommandBuffer(m_commandBuffers[i]);

			KR_CORE_ASSERT(resultCB == VK_SUCCESS, "Failed to record command buffer");
		}
	}

	void VulkanRendererAPI::EndScene()
	{
		vkDeviceWaitIdle(VulkanHolder::GetVulkanContext()->GetLogicalDevice());
		for (size_t i = 0; i < m_commandBuffers.size(); i++)
		{
			vkResetCommandBuffer(m_commandBuffers[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		}
		m_VulkaVertexArrays.clear();
	}

	void VulkanRendererAPI::CreateSynchronicity()
	{
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

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

	void VulkanRendererAPI::SubmitCommandBuffers()
	{
		vkWaitForFences(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult resultAI = vkAcquireNextImageKHR(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), VulkanHolder::GetVulkanContext()->GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (resultAI == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateCommandBuffersPipelineSwapchain();
		}
		else if (resultAI != VK_SUCCESS && resultAI != VK_SUBOPTIMAL_KHR)
		{
			KR_CORE_ASSERT(false, "Failed to acquire swapchain image");
		}

		VulkanHolder::GetVulkanContext()->UploadUBO(imageIndex);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

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
			RecreateCommandBuffersPipelineSwapchain();
		}
		else if (resultQP != VK_SUCCESS)
		{
			KR_CORE_ASSERT(false, "Failed to present swapchain image");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRendererAPI::RecreateCommandBuffersPipelineSwapchain()
	{
		vkDeviceWaitIdle(VulkanHolder::GetVulkanContext()->GetLogicalDevice());

		vkFreeCommandBuffers(VulkanHolder::GetVulkanContext()->GetLogicalDevice(), VulkanHolder::GetVulkanContext()->GetCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		m_bAllocateCommandBuffers = true;

		for (auto vulkanVA : m_VulkaVertexArrays)
		{
			vulkanVA->CleanupPipeline();
		}

		VulkanHolder::GetVulkanContext()->ClearUBO();
		VulkanHolder::GetVulkanContext()->RecreateSwapChain();
		VulkanHolder::GetVulkanContext()->RecreateUBO();

		for (auto vulkanVA : m_VulkaVertexArrays)
		{
			vulkanVA->RecreateVulkanVA();
		}

		AllocateCommandBuffers();
	}

	void VulkanRendererAPI::DrawIndexed(std::shared_ptr<VertexArray> vertexArray)
	{
		std::shared_ptr<VulkanVertexArray> vulkanVA = std::static_pointer_cast<VulkanVertexArray>(vertexArray);
		m_VulkaVertexArrays.push_back(vulkanVA);
		RecordCommandBuffers();
		SubmitCommandBuffers();
	}
}
