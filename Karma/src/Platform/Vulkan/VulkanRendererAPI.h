/**
 * @file VulkanRendererAPI.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains VulkanRendererAPI class with Vulkan specific implementation of RendererAPI pure virtual functions.
 * @version 1.0
 * @date Jan 16, 2021
 * 
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/RendererAPI.h"
#include "vulkan/vulkan.h"

namespace Karma
{
	class VulkanVertexArray;

	/**
	 * @brief Vulkan specific implementation of RendererAPI pure virtual functions.
	 * 
	 * @see RendererAPI
	 * @since Karma 1.0.0
	 */
	class KARMA_API VulkanRendererAPI : public RendererAPI
	{
	public:
		/**
		 * @brief A constructor
		 * 
		 * @note Vulkan specific resoures are allocated in VulkanContext::Init()
		 * @since Karma 1.0.0
		 */
		VulkanRendererAPI();

		/**
		 * @brief A destructor
		 * 
		 * @note Vulkan specific resources are deallocated in VulkanContext::~VulkanContext()
		 * @since Karma 1.0.0
		 */
		virtual ~VulkanRendererAPI();

		/**
		 * @brief Sets the color to be used for clear (rendering) screen
		 * 
		 * @param color				The color to be used for clearing
		 * @since Karma 1.0.0
		 */
		virtual void SetClearColor(const glm::vec4& color) override;

		/**
		 * @brief Clear the rendering screen. Not much use in Vulkan as such since we record commands in command buffers.
		 * 
		 * The commandbuffers are reset and begin command recording afresh each frame in RecordCommandBuffers() which includes clearing the screen.
		 * 
		 * @since Karma 1.0.0
		 */
		virtual void Clear() override;

		/**
		 * @brief Setting up resources for rendering of a scene which includes allocating command buffers if not already done 
		 * or when swapchain is rebuilt.
		 * 
		 * @see RenderCommand::BeginScene, VulkanRendererAPI::RecreateCommandBuffersAndSwapChain()
		 * @since Karma 1.0.0
		 */
		virtual void BeginScene() override;
		
		/**
		 * @brief Routine for drawing primitives
		 * 
		 * @param vertexArray						The format and relevant information of the rendrable vertex data
		 * 
		 * @see VulkanVertexArray
		 * @since Karma 1.0.0
		 */
		virtual void DrawIndexed(std::shared_ptr<VertexArray> vertexArray) override;

		/**
		 * @brief Instructions for end of the scene
		 * 
		 * @see RenderCommand::EndScene
		 * @since Karma 1.0.0
		 */
		virtual void EndScene() override;

		/**
		 * @brief Allocates resources for command buffers.
		 * 
		 * Command buffers are objects used to record commands which can be subsequently submitted to a device queue for execution.
		 * For instance the graphics and presentation queues.
		 * 
		 * @since Karma 1.0.0
		 */
		void AllocateCommandBuffers();

		/**
		 * @brief Records the command buffers with rendering commands for a particular image in the swapchain.
		 * 
		 * @param commandBuffer						The command buffer to be recorded
		 * @param imageIndex						The index of the swapchain image to record commands for
		 * 
		 * @see VulkanRendererAPI::SubmitCommandBuffers()
		 * @since Karma 1.0.0
		 */
		void RecordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		/**
		 * @brief Submits the recorded command buffers to the graphics queue for execution.
		 * 
		 * @see VulkanRendererAPI::DrawIndexed(), VulkanRendererAPI::RecordCommandBuffers()
		 * @since Karma 1.0.0
		 */
		void SubmitCommandBuffers();

		/**
		 * @brief Creates synchronization objects like semaphores and fences for coordinating rendering operations.
		 * 
		 * @since Karma 1.0.0
		 */
		void CreateSynchronicity();

		/**
		 * @brief Clears up VulkanRendererAPI specific resources like command buffers and synchronization objects.
		 * 
		 * @see VulkanContext::~VulkanContext()
		 * @since Karma 1.0.0
		 */
		void ClearVulkanRendererAPI();

		/**
		 * @brief Removes synchronization objects like semaphores and fences.
		 * 
		 * @see VulkanRendererAPI::ClearVulkanRendererAPI()
		 * @since Karma 1.0.0
		 */
		void RemoveSynchronicity();

		/**
		 * @brief Recreates command buffers, graphics pipelines, and uniform buffer objects when swapchain is 
		 * recreated (like on window resize when acquiring next image from swapchain gives VK_ERROR_OUT_OF_DATE_KHR).
		 * 
		 * @see VulkanRendererAPI::SubmitCommandBuffers()
		 * @since Karma 1.0.0
		 */
		void RecreateCommandBuffersPipelineSwapchain();

		/**
		 * @brief Recreates command buffers and swapchain when swapchain is recreated (like on window resize when acquiring next image from swapchain gives VK_ERROR_OUT_OF_DATE_KHR).
		 * 
		 * @note Seems like better to calls this than RecreateCommandBuffersPipelineSwapchain()
		 * 
		 * @see KarmaGuiRenderer::GiveLoopBeginControlToVulkan()
		 * @since Karma 1.0.0
		 */
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
