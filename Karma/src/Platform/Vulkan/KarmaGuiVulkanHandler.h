/**
 * @file KarmaGuiVulkanHandler.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains KarmaGuiVulkanHandler class and relevant data structures.
 * @version 1.0
 * @date August 30, 2022
 *
 * @copyright 2014-2023 Omar Cornut
 */
// Dear ImGui (1.89.2) is Copyright (c) 2014-2023 Omar Cornut.

// Major and heavy modifications of the original Dear ImGUI's take on Vulkan implementation
// https://github.com/ravimohan1991/imgui/blob/9068fd1afbaba56247aacd452bbadebb286800c7/backends/imgui_impl_vulkan.h
// Disclaimer:
// Most of the member variables and function naming in the herein is credited to the author(s) of Dear ImGUI. I take no responsibility
// of damage caused by short names herein.

#pragma once

#include "krpch.h"

#include "KarmaGui.h"
#include "KarmaGuiInternal.h"
#include "Karma/Renderer/Scene.h"
#include <vulkan/vulkan.h>

namespace Karma
{

	/**
	 * @brief Macro for vulkan's alignment needs.
	 *
	 * @note Memory align e.g. KR_MEMALIGN(0,4)=0, KR_MEMALIGN(1,4)=4, KR_MEMALIGN(4,4)=4, KR_MEMALIGN(5,4)=8
	 * @since Karma 1.0.0
	 */
	#define KR_MEMALIGN(_OFF,_ALIGN)        (((_OFF) + ((_ALIGN) - 1)) & ~((_ALIGN) - 1))

	/**
	 * @brief Macro for vulkan's maximum element estimation, out of the supplied two elements.
	 *
	 * @since Karma 1.0.0
	 */
	#define KR_MAX(A, B)    (((A) >= (B)) ? (A) : (B))

	/**
	 * @brief A data structure for Vulkan's initialization done in KarmaGuiRenderer::KarmaGui_ImplVulkan_Init()
	 *
	 * Mostly, with the exception of DescriptorPool, the handles are referring to the objects, instantiated by VulkanContext::Init() function, set via KarmaGuiRenderer::SetUpKarmaGuiRenderer
	 *
	 * @note Should be zero cleared before use
	 */
	struct KarmaGui_ImplVulkan_InitInfo
	{
		/**
		 * @brief The handle to Vulkan's instance object, for use in backend
		 *
		 * @since Karma 1.0.0
		 */
		VkInstance                      Instance;

		/**
		 * @brief The handle to the Vulkan's software representaion of GPU, for use in backend
		 *
		 * @since Karma 1.0.0
		 */
		VkPhysicalDevice                PhysicalDevice;

		/**
		 * @brief A logical device handle. Device is an appropriate instance using which swapchains, framebuffers, and all such Vulkan resources are instantiated and created.
		 * For use in backend.
		 *
		 * @since Karma 1.0.0
		 */
		VkDevice                        Device;

		/**
		 * @brief Graphics queue family (valued in unsigned integer) properties used for creating commandpool, swapchain, and logical device. For backend.
		 *
		 * For instance Queue family #0 supports transfer, graphics, compute, and presentation
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t                        QueueFamily;

		/**
		 * @brief Opaque handle to a queue object. For backend submission of queues via vkQueueSubmit.
		 *
		 * @since Karma 1.0.0
		 */
		VkQueue                         Queue;

		/**
		 * @brief A fresh descriptor pool created in KarmaGuiRenderer::CreateDescriptorPool() (seperate from VulkanVertexArray::CreateDescriptorPool()) for the uniforms and texture purposes. In this backend chiefly for KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_AddTexture routine.
		 *
		 * @since Karma 1.0.0
		 */
		VkDescriptorPool                DescriptorPool;

		/**
		 * @brief A handle to render pass object which represents a collection of attachments (depth attachment or colo(u)r attachment ), subpasses, and dependencies between the subpasses, and describes how the attachments are used over the course of the subpasses. Taken from the one created in VulkanContext::CreateRenderPass().
		 *
		 * @since Karma 1.0.0
		 */
		VkRenderPass                    RenderPass; // Very experimental here

		// begin Optional
		/**
		 * @brief  Usually this cache is set to VK_NULL_HANDLE, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that cache is enabled for the duration of the command.
		 *
		 * @since Karma 1.0.0
		 */
		VkPipelineCache                 PipelineCache;

		/**
		 * @brief Not sure what the sue of this Subpass is. Seems vestigial?
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t                        Subpass;
		// end Optional

		/**
		 * @brief The minimum number of images the specified device supports for a swapchain created for the surface, and will be at least one plus one.
		 *
		 * @note Taken from VulkanContext and supposed to be >= 2.
		 * @since Karma 1.0.0
		 */
		uint32_t                        MinImageCount;

		/**
		 * @brief The number of elements in the pSwapchainImages array, i.e number of presentable swapchain images available (vkGetSwapchainImagesKHR).
		 *
		 * @note Taken from VulkanContext where (VulkanContext::CreateSwapChain()) ImageCount is defined MinImageCount + 1.
		 * @since Karma 1.0.0
		 */
		uint32_t                        ImageCount;

		/**
		 * @brief Is a VkSampleCountFlagBits value specifying the number of samples used in rasterization. This value is ignored for the purposes of setting the number of samples used in rasterization if the pipeline is created with the VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT dynamic state set, but if VK_DYNAMIC_STATE_SAMPLE_MASK_EXT dynamic state is not set, it is still used to define the size of the pSampleMask array.
		 *
		 * @note >= VK_SAMPLE_COUNT_1_BIT (0 -> default to VK_SAMPLE_COUNT_1_BIT)
		 * @since Karma 1.0.0
		 */
		VkSampleCountFlagBits           MSAASamples;

		/**
		 * @brief A pointer to a valid VkAllocationCallbacks structure. Structure containing callback function pointers for memory allocation
		 *
		 * @note Since Dear ImGui's implementation is using this technique, I am just not interfering where I'd be using NULL.
		 * @since Karma 1.0.0
		 */
		const VkAllocationCallbacks* Allocator;
	};

	//-------------------------------------------------------------------------
	// Internal / Miscellaneous Vulkan Helpers
	// (Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT used by your own engine/app.)
	//-------------------------------------------------------------------------
	// You probably do NOT need to use or care about those functions.
	// Those functions only exist because:
	//   1) they facilitate the readability and maintenance of the multiple main.cpp examples files.
	//   2) the multi-viewport / platform window implementation needs them internally.
	// Generally we avoid exposing any kind of superfluous high-level helpers in the bindings,
	// but it is too much code to duplicate everywhere so we exceptionally expose them.
	//
	// Your engine/app(lication) will likely _already_ have code to setup all that stuff (swap chain, render pass, frame buffers, etc.).
	// You may read this code to learn about Vulkan, but it is recommended you use you own custom tailored code to do equivalent work.
	// (The ImGui_ImplVulkanH_XXX functions do not interact with any of the state used by the regular ImGui_ImplVulkan_XXX 	functions)
	//-------------------------------------------------------------------------

	// Cowboy's confusion clarification concept!
	// It seems the ImGUI author(s) have mixed and/or confused notion of
	// ImageCount, which decides the number of SwapChainImages, framebuffer, and so on (contained within
	// ImGui_ImplVulkanH_ImageFrame structure).
	// (https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Retrieving-the-swap-chain-images)
	// and MAX_FRAMES_IN_FLIGHT, which is representative of (linearly proportional to or indicative of) number of commandbuffer recordings on CPU that may happen whilst the rendering is being done on GPU. That should determine the semaphore, fence, and commandbuffer size.
	// https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Frames_in_flight
	// The argument is elicited by the comment line https://github.com/ravimohan1991/imgui/blob/e4967701b67edd491e884632f239ab1f38867d86/backends/imgui_impl_vulkan.h#L144

	/**
	 * @brief Data structure for synchronous operations (relevant to rendering in this context).
	 *
	 * @note It seems the ImGUI author(s) have mixed and/or confused notion of
	 * ImageCount, which decides the number of SwapChainImages, framebuffer, and so on (contained within
	 * KarmaGui_ImplVulkanH_ImageFrame structure)
	 * (https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Retrieving-the-swap-chain-images)
	 * and MAX_FRAMES_IN_FLIGHT, which is representative of (linearly proportional to or indicative of) number of commandbuffer recordings on CPU that may happen whilst the rendering is being done on GPU. That should determine the semaphore, fence, and commandbuffer size.
	 * https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Frames_in_flight
	 * The argument is elicited by the comment line https://github.com/ravimohan1991/imgui/blob/e4967701b67edd491e884632f239ab1f38867d86/backends/imgui_impl_vulkan.h#L144
	 * @since Karma 1.0.0
	 */
	struct KarmaGui_Vulkan_Frame_On_Flight
	{
		/**
		 * @brief A handle to fence object
		 *
		 * Fences are a synchronization primitive that can be used to insert a dependency from a queue to the host. 
		 * Fences have two states - signaled and unsignaled. A fence can be signaled as part of the execution of a queue submission command.
		 * Fences can be unsignaled on the host with vkResetFences. Fences can be waited on by the host with the vkWaitForFences command, and the
		 * current state can be queried with vkGetFenceStatus.
		 *
		 * @since Karma 1.0.0
		 */
		VkFence             Fence;

		/**
		 * @brief Handle to semaphore object for next available presentable image (vkAcquireNextImageKHR).
		 *
		 * Semaphores are a synchronization primitive that can be used to insert a dependency between queue operations or between a queue operation and the host.
		 * Binary semaphores have two states - signaled and unsignaled. Timeline semaphores have a strictly increasing 64-bit unsigned integer payload and are signaled with respect
		 * to a particular reference value. A semaphore can be signaled after execution of a queue operation is completed, and a queue operation can wait for a semaphore to become
		 * signaled before it begins execution. A timeline semaphore can additionally be signaled from the host with the vkSignalSemaphore command and waited on from the host
		 * with the vkWaitSemaphores command.
		 *
		 * @since Karma 1.0.0
		 */
		VkSemaphore         ImageAcquiredSemaphore;

		/**
		 * @brief Handle to semaphore to wait for before issuing the present request (vkQueuePresentKHR).
		 *
		 * Semaphores are a synchronization primitive that can be used to insert a dependency between queue operations or between a queue operation and the host.
		 * Binary semaphores have two states - signaled and unsignaled. Timeline semaphores have a strictly increasing 64-bit unsigned integer payload and are signaled with respect
		 * to a particular reference value. A semaphore can be signaled after execution of a queue operation is completed, and a queue operation can wait for a semaphore to become
		 * signaled before it begins execution. A timeline semaphore can additionally be signaled from the host with the vkSignalSemaphore command and waited on from the host
		 * with the vkWaitSemaphores command.
		 * 
		 * @since Karma 1.0.0
		 */
		VkSemaphore         RenderCompleteSemaphore;

		/**
		 * @brief Object used to record commands which can be subsequently submitted to a device queue for execution.
		 *
		 * @since Karma 1.0.0
		 */
		VkCommandBuffer     CommandBuffer;
	};

	/**
	 * @brief Helper structure to hold the data needed by one rendering ImageFrame (different from the FRAME_IN_FLIGHT frame!)
	 * (Used by multi-viewport features. We gonna just use this.). The number of frames is MinImageCount = m_MinImageCount + 1.
	 *
	 * @note Please zero-clear before use!
	 *
	 * @since Karma 1.0.0
	 */
	struct KarmaGui_ImplVulkanH_ImageFrame
	{
		/**
		 * @brief VulkanContext m_swapChainImages equivalent. The number of images (MinImageCount = m_MinImageCount + 1) recordings on CPU that may happen whilst 
		 * the rendering is being done on GPU.
		 *
		 * @since Karma 1.0.0
		 */
		VkImage             Backbuffer;

		/**
		 * @brief VulkanContext m_swapChainImageViews equivalent. Again, the number is same.
		 *
		 * @since Karma 1.0.0
		 */
		VkImageView         BackbufferView;

		/**
		 * @brief A handle to framebuffer which represents a collection of specific memory attachments that a render pass instance uses.
		 *
		 * @since Karma 1.0.0
		 */
		VkFramebuffer       Framebuffer;
	};

	/**
	 * @brief Helper structure to hold the data needed by one rendering context (Vulkan) of one OS window (Windows, Mac, or Linux).
	 *
	 * @since Karma 1.0.0
	 */
	struct KarmaGui_ImplVulkanH_Window
	{
		/**
		 * @brief Width dimension of the Window
		 *
		 * @note Used in KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_RenderWindow which is not used in current state of the Engine.
		 *
		 * @since Karma 1.0.0
		 */
		int                 Width;

		/**
		 * @brief Height dimension of the Window
		 *
		 * @note Used in KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_RenderWindow which is not used in current state of the Engine.
		 *
		 * @since Karma 1.0.0
		 */
		int                 Height;

		/**
		 * @brief Vulkan does not have the concept of a "default framebuffer", hence it requires an infrastructure that will own the buffers we will render to before we visualize them on the screen. This infrastructure is known as the swap chain and must be created explicitly in Vulkan. The swap chain is essentially a queue of images that are waiting to be presented to the screen. Our backend will acquire such an image to draw to it, and then return it to the queue. Taken from VulkanContext::CreateSwapChain().
		 *
		 * @since Karma 1.0.0
		 */
		VkSwapchainKHR      Swapchain;

		/**
		 * @brief Command pools manage the memory that is used to store the buffers and command buffers are allocated from them. Taken from VulkanContext::CreateCommandPool()
		 *
		 * @since Karma 1.0.0
		 */
		VkCommandPool       CommandPool;

		/**
		 * @brief The surface is a cross-platform abstraction over windows to render to and is generally instantiated by providing a reference to the native window handle
		 * @note The vulkan backend, in current stance, is not using the surface created in VulkanContext::CreateSurface() since all the surface relevant operations have already been done in VulkanContext.
		 *
		 * @since Karma 1.0.0
		 */
		VkSurfaceKHR        Surface;

		/**
		 * @brief Structure describing a supported swapchain format-color space pair. Taken from VulkanContext::CreateSwapChain().
		 *
		 * @since Karma 1.0.0
		 */
		VkSurfaceFormatKHR  SurfaceFormat;

		/**
		 * @brief Presentation mode supported for a surface. Taken from VulkanContext::CreateSwapChain().
		 *
		 * @since Karma 1.0.0
		 */
		VkPresentModeKHR    PresentMode;

		/**
		 * @brief A render pass object represents a collection of attachments, subpasses, and dependencies between the subpasses, and describes how the attachments are used over the course of the subpasses.
		 *
		 * @since Karma 1.0.0
		 */
		VkRenderPass        RenderPass;

		/**
		 * @brief A huge object in Vulkan that encompasses the configuration of the entire GPU for the draw. Once a pipeline is built, it can be bound inside a command buffer, and then when you draw anything it will use the bound pipeline.
		 *
		 * @note The window pipeline may use a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo. Need to investigate why.
		 * @since Karma 1.0.0
		 */
		VkPipeline          Pipeline;

		/**
		 * @brief No clue about the usage
		 *
		 * @since Karma 1.0.0
		 */
		bool                ClearEnable;

		/**
		 * @brief VkClearValue structures containing clear values for each attachment, if the attachment uses a loadOp value of VK_ATTACHMENT_LOAD_OP_CLEAR or if the attachment has a depth/stencil format and uses a stencilLoadOp value of VK_ATTACHMENT_LOAD_OP_CLEAR. The array is indexed by attachment number. Only elements corresponding to cleared attachments are used. Other elements of pClearValues are ignored.
		 *
		 * @since Karma 1.0.0
		 */
		VkClearValue        ClearValue;

		/**
		 * @brief The number count of the image, returned by vkGetSwapchainImagesKHR, usually range derived from min_image_count, to be addressed for frame (each loop iteration) rendering logic.
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t            ImageFrameIndex;

		/**
		 * @brief Total Number of the images supported by swapchain.
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t            TotalImageCount;

		/**
		 * @brief Current set of swapchain wait semaphores and command buffers we're using (0 <= SemaphoreIndex < MAX_FRAMES_IN_FLIGHT)
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t            SemaphoreIndex;

		/**
		 * @brief Is representative of (linearly proportional to or indicative of) number of commandbuffer recordings on CPU that may happen whilst the rendering is being done on GPU. That should determine the semaphore, fence, and commandbuffer size.
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t                 MAX_FRAMES_IN_FLIGHT;
		
		/**
		 * @brief Just a container for buffers and all those sizes depending on  VulkanHolder::GetVulkanContext()->GetSwapChainImages().size();
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplVulkanH_ImageFrame* ImageFrames;

		// Cowboy's Modification

		/**
		 * @brief Data for synchronous operations of in-flight rendering frames.
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_Vulkan_Frame_On_Flight* FramesOnFlight;

		/**
		 * @brief Most likely the glfw window size, set from VulkanHolder::GetVulkanContext()->GetSwapChainExtent(). Used for setting renderPassInfo.renderArea.extent in KarmaGuiRenderer::FrameRender.
		 *
		 * @todo Ponder, how to implement the context switch from Editor rendering to Game rendering.
		 *
		 * @since Karma 1.0.0
		 */
		VkRect2D                            RenderArea;

		/**
		 * @brief Constructor for zero clearence and relevant stuff.
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplVulkanH_Window()
		{
			memset((void*)this, 0, sizeof(*this));
			PresentMode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
			ClearEnable = true;
		}
	};

	/**
	 * @brief Reusable buffers used for rendering 1 current in-flight ImageFrame, for KarmaGui_ImplVulkan_RenderDrawData().
	 * Seems like data structure with single instantiation for each of the FrameIndex.
	 *
	 * @note Please zero-clear before use.
	 */
	struct KarmaGui_ImplVulkanH_ImageFrameRenderBuffers
	{
		/**
		 * @brief Device (KarmaGui_ImplVulkan_InitInfo::Device) memory allocated via vkAllocateMemory for containing vertexbuffer.
		 *
		 * For instance, the filling is done like so
		 *@code{.cpp}
		 *	vkMapMemory(vulkanInfo->Device, renderBuffer->VertexBufferMemory, 0, renderBuffer->VertexBufferSize, 0, (void**)(&vertexData));
		 *@endcode
		 *
		 * @since Karma 1.0.0
		 */
		VkDeviceMemory      VertexBufferMemory;

		/**
		 * @brief Device (KarmaGui_ImplVulkan_InitInfo::Device) memory allocated via vkAllocateMemory for containing indexbuffer
		 *
		 * For instance, the filling is done like so
		 *@code{.cpp}
		 *	vkMapMemory(vulkanInfo->Device, renderBuffer->IndexBufferMemory, 0, renderBuffer->IndexBufferSize, 0, (void**)(&indexData));
		 *@endcode
		 *
		 * @since Karma 1.0.0
		 */
		VkDeviceMemory      IndexBufferMemory;

		/**
		 * @brief The size in bytes of the vertex buffer.
		 *
		 * @since Karma 1.0.0
		 */
		VkDeviceSize        VertexBufferSize;

		/**
		 * @brief The size in bytes of the index buffer.
		 *
		 * @since Karma 1.0.0
		 */
		VkDeviceSize        IndexBufferSize;

		/**
		 * @brief Vulkan buffer to be bound to KarmaGui_Vulkan_Frame_On_Flight::CommandBuffer for vertexbuffer
		 *
		 * @since Karma 1.0.0
		 */
		VkBuffer            VertexBuffer;

		/**
		 * @brief Vulkan buffer to be bound to KarmaGui_Vulkan_Frame_On_Flight::CommandBuffer for indexbuffer
		 *
		 * @since Karma 1.0.0
		 */
		VkBuffer            IndexBuffer;

	public:

		/**
		 * @brief A constructor for zelo initialization and relevant assignment.
		 *
		 * @since Karma 1.0..0
		 */
		KarmaGui_ImplVulkanH_ImageFrameRenderBuffers()
		{
			VertexBufferSize = IndexBufferSize = 0;
			IndexBuffer = VertexBuffer = VK_NULL_HANDLE;
			IndexBufferMemory = VertexBufferMemory = VK_NULL_HANDLE;
		}
	};

	/**
	 * @brief A data structure for packaging of rendering buffers allocated for a current in-flight ImageFrame.
	 *
	 * @note We are not supporting multi-viewports in the current state of Engine. Also please zero-clear before use
	 * @since Karma 1.0.0
	 */
	struct KarmaGui_ImplVulkanH_WindowRenderBuffers
	{
		/**
		 * @brief Current KarmaGui_ImplVulkanH_Window::SemaphoreIndex
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t            Index;

		/**
		 * @brief Basically MAX_FRAMES_IN_FLIGHT, representative of total number of KarmaGui_ImplVulkanH_ImageFrameRenderBuffers to be allocated for rendering
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t            Count;

		/**
		 * @brief Reusable buffers used for rendering, to be instantiated.
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplVulkanH_ImageFrameRenderBuffers* FrameRenderBuffers;
	};

	/**
	 * @brief Data relevant to rendering of a Mesa viewport.
	 *
	 * Helper structure we store in the void* RenderUserData field of each KarmaGuiViewport to easily retrieve our backend data.
	 *
	 * @note We are not supporting multiviewport in the current state of Engine.
	 * @since Karma 1.0.0
	 */
	struct KarmaGui_ImplVulkan_ViewportData
	{
		/**
		 * @brief Probabbly for multiview port which we are not supporting in the Current state.
		 *
		 * @since Karma 1.0.0
		 */
		bool                                    WindowOwned;

		/**
		 * @brief Window data to be used by secondary viewports only.
		 *
		 * @note We are not supporting multi viewports in the current state of the Engine
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplVulkanH_Window                Window;

		/**
		 * @brief The rendering relevant buffers to be used by viewport.
		 *
		 * Basically to be used by any viewport but since we are only supporting single viewport, mindset must be done accordingly.
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplVulkanH_WindowRenderBuffers   RenderBuffers;
		//ImGui_ImplVulkanH_Window*               ImGUILayerWindowData;

		/**
		 * @brief A constructor for allocation and 0 initialization
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplVulkan_ViewportData()
		{
			WindowOwned = false;
			memset(&RenderBuffers, 0, sizeof(RenderBuffers));
		}

		/**
		 * @brief A destructor
		 *
		 * @since Karma 1.0.0
		 */
		~KarmaGui_ImplVulkan_ViewportData() { }
	};

	/**
	 * @brief Data structure containing the Mesa image texture relevant data. For instance, file and folder icons of Content Browser.
	 *
	 * Similar to MesaDecalData in KarmaGuiOpenGLHandler.
	 *
	 * @since Karma 1.0.0
	 */
	struct KarmaGui_ImplVulkan_Image_TextureData
	{
		/**
		 * @brief The  name of the image texture for Mesa
		 *
		 * @note Ponder over similar member in MesaDecalData.
		 * @since Karma 1.0.0
		 */
		std::string                 TextureLable;

		/**
		 * @brief 2D width of the texture.
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t                    width;

		/**
		 * @brief 2D height of the texture.
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t                    height;

		/**
		 * @brief The number of image components in image texture.
		 *
		 * @see KarmaUtilities::GetImagePixelData
		 * @since Karma 1.0.0
		 */
		uint32_t                    channels;

		/**
		 * @brief The size, in bytes, of the image texture
		 *
		 * Formula:
		 * @code{.cpp}
		 *  width * height * 4 * sizeof(char)
		 * @endcode
		 *
		 * @since Karma 1.0.0
		 */
		size_t                      size;

		/**
		 * @brief This is a sampler handle, and is used in descriptor updates for types VK_DESCRIPTOR_TYPE_SAMPLER and VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER if the binding being updated does not use immutable samplers.
		 *
		 * @since Karma 1.0.0
		 */
		VkSampler                   TextureSampler;

		/**
		 * @brief A VkDeviceMemory object describing the device memory, relevant to the image, to attach.
		 *
		 * @since Karma 1.0.0
		 */
		VkDeviceMemory              TextureMemory;

		/**
		 * @brief The actual vulkan's image object.
		 *
		 * @since Karma 1.0.0
		 */
		VkImage                     TextureImage;

		/**
		 * @brief The image view representing contiguous ranges of the image subresources and containing additional metadata which are used by pipeline shaders for reading or writing image data.
		 *
		 * @since Karma 1.0.0
		 */
		VkImageView                 TextureView;

		/**
		 * @brief Container for uploading image texture relevant data, TextureView for instance, from CPU to GPU
		 *
		 * @since Karma 1.0.0
		 */
		VkDescriptorSet             TextureDescriptorSet;

		/**
		 * @brief Device allocated memory for image pixels' buffer.
		 *
		 * @since Karma 1.0.0
		 */
		VkDeviceMemory              UploadBufferMemory;

		/**
		 * @brief Vulkan buffer containing texture pixels
		 *
		 * @since Karma 1.0.0
		 */
		VkBuffer                    UploadBuffer;

		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplVulkan_Image_TextureData()
		{
			TextureLable = "NO LABLE YET";
		}
	};

	/**
	 * @brief Backend data containing stuff for Vulkan renderer for KarmaGui.
	 *
	 * The object is accessed by the acessor function KarmaGuiRenderer::GetBackendRendererUserData()
	 *
	 * @note This class is base for KarmaGuiBackendRendererUserData which is fat and needs rethinking from polymorphism pov. Hence also in todo list.
	 * @todo Ponder upon the fatness.
	 *
	 * @since Karma 1.0.0
	 */
	struct KarmaGui_ImplVulkan_Data
	{
		/**
		 * @brief KarmaGui's initialization data
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplVulkan_InitInfo   VulkanInitInfo;

		/**
		 * @brief A render pass object represents a collection of attachments, subpasses, and dependencies between the subpasses, and describes how the attachments are used over the course of the subpasses.
		 *
		 * Set to KarmaGui_ImplVulkan_InitInfo.RenderPass in the function KarmaGuiRenderer::KarmaGui_ImplVulkan_Init
		 *
		 * @since Karma 1.0.0
		 */
		VkRenderPass                RenderPass;

		/**
		 * @brief The value for default alignment for index and vertex buffers. The value is set to 256 by DearImgui authors, so we do in KarmaGuiRenderer::KarmaGui_ImplVulkan_Init.
		 *
		 * @see See Align
		 * @since Karma 1.0.0
		 */
		VkDeviceSize                BufferMemoryAlignment;

		/**
		 * @brief Flags to be set for Vulkan's graphics pipeline.
		 *
		 * @since Karma 1.0.0
		 */
		VkPipelineCreateFlags       PipelineCreateFlags;

		/**
		 * @brief A handle to the descriptorsetlayout created by backend in KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateDescriptorSetLayout
		 *
		 * A descriptor set layout object is defined by an array of zero or more descriptor bindings. Each individual descriptor binding is specified by a descriptor type, a count (array size) of the number of descriptors in the binding, a set of shader stages that can access the binding, and (if using immutable samplers) an array of sampler descriptors.
		 *
		 * In this backend context, the layout is created for fonts to be programmed? in fragment shader stage.
		 *
		 * @since Karma 1.0.0
		 */
		VkDescriptorSetLayout       DescriptorSetLayout;

		/**
		 * @brief Opaque handle to a pipeline layout object. Access to descriptor sets from a pipeline is accomplished through a pipeline layout. Zero or more descriptor set layouts and zero or more push constant ranges are combined to form a pipeline layout object describing the complete set of resources that can be accessed by a pipeline. The pipeline layout represents a sequence of descriptor sets with each having a specific layout. This sequence of layouts is used to determine the interface between shader stages and shader resources. Each pipeline is created using a pipeline layout.
		 *
		 * @see KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreatePipelineLayout
		 * @since Karma 1.0.0
		 */
		VkPipelineLayout            PipelineLayout;

		/**
		 * @brief A huge object in Vulkan that encompasses the configuration of the entire GPU for the draw. Once a pipeline is built, it can be bound inside a command buffer, and then when you draw anything it will use the bound pipeline.
		 *
		 * @since Karma 1.0.0
		 */
		VkPipeline                  Pipeline;

		/**
		 * @brief Not sure what the use of this Subpass is. Seems vestigial?
		 *
		 * @since Karma 1.0.0
		 */
		uint32_t                    Subpass;

		/**
		 * @brief Opaque handle to a vertex shader module object for backend. %Shader modules contain shader code and one or more entry points. Shaders are selected from a shader module by specifying an entry point as part of pipeline creation. The stages of a pipeline can use shaders that come from different modules. The shader code defining a shader module must be in the SPIR-V format, as described by the Vulkan Environment for SPIR-V appendix.
		 *
		 * @since Karma 1.0.0
		 */
		VkShaderModule              ShaderModuleVert;

		/**
		 * @brief Opaque handle to fragment shader module object for backend. %Shader modules contain shader code and one or more entry points. Shaders are selected from a shader module by specifying an entry point as part of pipeline creation. The stages of a pipeline can use shaders that come from different modules. The shader code defining a shader module must be in the SPIR-V format, as described by the Vulkan Environment for SPIR-V appendix.
		 *
		 * @since Karma 1.0.0
		 */
		VkShaderModule              ShaderModuleFrag;

		// Font data
		/**
		 * @brief Opaque handle to a sampler object. The object represents the state of an image sampler which is used by the implementation to read image data and apply filtering and other transformations for the shader.
		 *
		 * @since Karma 1.0.0
		 */
		VkSampler                   FontSampler;

		/**
		 * @brief The memory resource allocated for the fonts to be displayed by the backend
		 *
		 * @since Karma 1.0.0
		 */
		VkDeviceMemory              FontMemory;

		/**
		 * @brief A 2D font image object with following properties
		 *
		 * 1. VK_SHARING_MODE_EXCLUSIVE: specifies that access to any range or image subresource of the object will be exclusive to a single queue family at a time.
		 * 2. VK_IMAGE_USAGE_TRANSFER_DST_BIT: specifies that the image can be used as the destination of a transfer command.
		 * 3. VK_IMAGE_USAGE_SAMPLED_BIT: specifies that the image can be used to create a VkImageView suitable for occupying a VkDescriptorSet slot either of type VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE or VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, and be sampled by a shader.
		 * 4. VK_IMAGE_TILING_OPTIMAL: specifies optimal tiling (texels are laid out in an implementation-dependent arrangement, for more efficient memory access).
		 * 5. VK_FORMAT_R8G8B8A8_UNORM specifies a four-component, 32-bit unsigned normalized format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, an 8-bit B component in byte 2, and an 8-bit A component in byte 3.
		 *
		 * @note The image is going to be used as destination for the buffer copy, so it should be set up as a transfer destination VK_IMAGE_USAGE_TRANSFER_DST_BIT. We also want to be able to access the image from the shader to color our mesh, so the usage should include VK_IMAGE_USAGE_SAMPLED_BIT. Refer https://vulkan-tutorial.com/Texture_mapping/Images
		 *
		 * @since Karma 1.0.0
		 */
		VkImage                     FontImage;

		/**
		 * @brief The image view, for 2D font, representing contiguous ranges of the image subresources and containing additional metadata which are used by pipeline shaders for reading or writing image data.
		 *
		 * @since Karma 1.0.0
		 */
		VkImageView                 FontView;

		/**
		 * @brief Container for uploading image texture relevant data, TextureView for instance, from CPU to GPU
		 *
		 * @since Karma 1.0.0
		 */
		VkDescriptorSet             FontDescriptorSet;

		/**
		 * @brief Device allocated memory for font image pixels' buffer.
		 *
		 * @since Karma 1.0.0
		 */
		VkDeviceMemory              UploadBufferMemory;

		/**
		 * @brief Vulkan buffer containing font texture pixels
		 *
		 * @since Karma 1.0.0
		 */
		VkBuffer                    UploadBuffer;

		// Image data
		/**
		 * @brief List of vulkan images' textures, for instance icons and stuff.
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<KarmaGui_ImplVulkan_Image_TextureData*>         vulkanMesaDecalDataList;

		// Render buffers for main window
		//KarmaGui_ImplVulkanH_WindowRenderBuffers MainWindowRenderBuffers;
		
		// heh, not working with initializers in MSVC. Need a blog post!
		
		/*
		ImGui_ImplVulkan_Data()
		{
			memset((void*)this, 0, sizeof(*this));
			BufferMemoryAlignment = 256;
		}*/
	};

	/**
	 * @brief A class for required Vulkan routines for KarmaGui Vulkan backend
	 */
	class KARMA_API KarmaGuiVulkanHandler
	{
	public:
		/*
		inline static KarmaGui_ImplVulkan_Data* KarmaGui_ImplVulkan_GetBackendData()
		{
			return KarmaGui::GetCurrentContext() ? (KarmaGui_ImplVulkan_Data*)KarmaGui::GetIO().BackendRendererUserData : nullptr;
		}*/
		// GetIO should fetche the configuration settings and whatnot, which in this case is the struct KarmaGui_ImplVulkan_Data

		/**
		 * @brief Reports memory relevant type information for the specified physical device (the GPU, represented by KarmaGui_ImplVulkan_InitInfo.PhysicalDevice), based on supplied properties and type_bits (not exceeding a valid limit).
		 *
		 * Device memory is memory that is visible to the device — for example the contents of the image or buffer objects, which can be natively used by the device.
		 *
		 * @param properties								The memory properties to be queried. This is a bitmask type for setting a mask of zero or more VkMemoryPropertyFlagBits.
		 * @param type_bits									This is a bitmask and contains one bit set for every supported memory type (obtained from vkGetBufferMemoryRequirements, for instance) for the resource. Bit i is set if and only if the memory type i in the VkPhysicalDeviceMemoryProperties structure for the physical device is supported for the resource.
		 *
		 * @return the appropriate device memory type, else uint32_t(0xFFFFFFFF)
		 * @since Karma 1.0.0
		 */
		static uint32_t KarmaGui_ImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits);

		/**
		 * @brief Creates new vulkan buffer and allocates appropriate memory based upon the supplied newSize (appropriately aligned) and usage.
		 *
		 * @param buffer										The vulkan buffer to be resized
		 * @param bufferMemory									The device memory to be used for buffer
		 * @param pBufferSize									The size, in bytes, of the memory resource alloted to the buffer
		 * @param newSize										The (could be unaligned?) size, in bytes, of the buffer to be created
		 * @param usage											This is a bitmask of VkBufferUsageFlagBits specifying allowed usages of the buffer.
		 *
		 * @note If supplied buffer and bufferMemory are not null, then they are destroyed.
		 * @since Karma 1.0.0
		 */
		static void CreateOrResizeBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize& pBufferSize, size_t newSize, VkBufferUsageFlagBits usage);

		/**
		 * @brief A routine to bind index/vertex buffers, setup a external viewport, and bind pipeline. Usually for rendering of KarmaGui windows and all that.
		 *
		 * @param drawData										All draw data to render a KarmaGui frame
		 * @param pipeline										The vulkan pipeline (created in KarmaGui_ImplVulkan_CreatePipeline) being used by the backend
		 * @param commandBuffer									The vulkan commandbuffer (taken from VulkanRendererAPI.m_CommandBuffer) being used by the backend, see todo list.
		 * @param remderingBufferData							The set of buffers relevant to current frame (in flight?)
		 * @param width											The viewport width size
		 * @param height										The viewport height size
		 *
		 *
		 * @todo Ponder over the commandBuffer usage (should backend have seperate commandbuffer?)
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_SetupRenderState(KGDrawData* drawData, VkPipeline pipeline, VkCommandBuffer commandBuffer, KarmaGui_ImplVulkanH_ImageFrameRenderBuffers* remderingBufferData, int width, int height);
		
		// Experimental
		/**
		 * @brief A routine to bind index/vertex buffers, setup a external viewport, and bind pipeline. Specifically for 3D rendering in KarmaGUI's window for (experimental) 3D exhibition.
		 *
		 * @param drawData										All draw data to render a KarmaGui frame
		 * @param pipeline										The vulkan pipeline (created in KarmaGui_ImplVulkan_CreatePipeline) being used by the backend
		 * @param commandBuffer									The vulkan commandbuffer (taken from VulkanRendererAPI.m_CommandBuffer) being used by the backend, see todo list.
		 * @param remderingBufferData							The set of buffers relevant to current frame (in flight?)
		 * @param width											The viewport width size
		 * @param height										The viewport height size
		 *
		 *
		 * @todo Ponder over the commandBuffer usage (should backend have seperate commandbuffer?)
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_SetupRenderStateFor3DRendering(Scene* sceneToDraw, VkCommandBuffer commandBuffer, KGDrawData* drawData);
		
		/**
		 * @brief A routine to create the shader modules (KarmaGui_ImplVulkan_Data.ShaderModuleVert and KarmaGui_ImplVulkan_Data.ShaderModuleFrag) for the backend GraphicsPipeline (KarmaGui_ImplVulkan_Data.Pipeline).
		 *
		 * @param device										A logical device handle. See KarmaGui_ImplVulkan_InitInfo.Device
		 * @param allocator										The controller of host memory allocation as described in the Memory Allocation chapter. See KarmaGui_ImplVulkan_InitInfo.Allocator
		 *
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_CreateShaderModules(VkDevice device, const VkAllocationCallbacks* allocator);

		/**
		 * @brief Bilinear sampling object is created for font and stored in KarmaGui_ImplVulkan_Data.FontSampler if not done already in KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateDeviceObjects().
		 *
		 * @param device										A logical device handle. See KarmaGui_ImplVulkan_InitInfo.Device
		 * @param allocator										The controller of host memory allocation as described in the Memory Allocation chapter. See KarmaGui_ImplVulkan_InitInfo.Allocator
		 *
		 *  @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_CreateFontSampler(VkDevice device, const VkAllocationCallbacks* allocator);

		/**
		 * @brief Creates KarmaGui_ImplVulkan_Data.DescriptorSetLayout if not already in KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateDeviceObjects()
		 *
		 * @param device										A logical device handle. See KarmaGui_ImplVulkan_InitInfo.Device
		 * @param allocator										The controller of host memory allocation as described in the Memory Allocation chapter. See KarmaGui_ImplVulkan_InitInfo.Allocator
		 *
		 *  @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_CreateDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks* allocator);

		/**
		 * @brief Creates KarmaGui_ImplVulkan_Data.PipelineLayout (if not already in KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_CreateDeviceObjects()).
		 *
		 * @param device										A logical device handle. See KarmaGui_ImplVulkan_InitInfo.Device
		 * @param allocator										The controller of host memory allocation as described in the Memory Allocation chapter. See KarmaGui_ImplVulkan_InitInfo.Allocator
		 *
		 *  @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_CreatePipelineLayout(VkDevice device, const VkAllocationCallbacks* allocator);

		/**
		 * @brief Creates KarmaGui_ImplVulkan_Data.Pipeline.
		 *
		 * @param device										A logical device handle. See KarmaGui_ImplVulkan_InitInfo.Device
		 * @param allocator										The controller of host memory allocation as described in the Memory Allocation chapter. See KarmaGui_ImplVulkan_InitInfo.Allocator
		 * @param pipelineCache									Usually this cache (KarmaGui_ImplVulkan_InitInfo.PipelineCache) is set to VK_NULL_HANDLE, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that cache is enabled for the duration of the command.
		 * @param renderPass									A render pass object (KarmaGui_ImplVulkan_Data.RenderPass) represents a collection of attachments, subpasses, and dependencies between the subpasses, and describes how the attachments are used over the course of the subpasses.
		 * @param MSAASamples									KarmaGui_ImplVulkan_InitInfo.MSAASamples
		 * @param pipeline										KarmaGui_ImplVulkan_Data.Pipeline
		 * @param subpass										KarmaGui_ImplVulkan_Data.subpass
		 *
		 *  @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_CreatePipeline(VkDevice device, const VkAllocationCallbacks* allocator, VkPipelineCache pipelineCache, VkRenderPass renderPass, VkSampleCountFlagBits MSAASamples, VkPipeline* pipeline, uint32_t subpass);

		/**
		 * @brief This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously. NOT SUPPORTED
		 *
		 * @note Something to strive for? :)
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_CreateWindow(KarmaGuiViewport* viewport);

		// Overload for destroying KarmaGui window
		/**
		 * @brief Destroys the KarmaGui window relevant data (KarmaGui_ImplVulkan_ViewportData) and buffers. Basically set to KarmaGuiPlatformIO::Renderer_DestroyWindow pointer.
		 *
		 * @param viewport										The current platform window to be destroyed
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_DestroyWindow(KarmaGuiViewport* viewport);

		/**
		 * @brief Destroys the KarmaGui window relevant data (KarmaGui_ImplVulkan_ViewportData) and buffers. Called by KarmaGuiRenderer::CleanUpVulkanAndWindowData()
		 *
		 * @see KarmaGuiVulkanHandler::ClearVulkanWindowData
		 *
		 * @param windowData									The vulkan context containing relevant data to be deallocated
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_DestroyWindow(KarmaGui_ImplVulkanH_Window* windowData);

		/**
		 * @brief Set the size of window rendering relevant resources (swapchain, framebuffers, and all that). Basically implementation of KarmaGuiPlatformIO::Renderer_SetWindowSize.
		 *
		 * @param viewport										The current platform window to be resized
		 * @param size											The 2D vector containing width and height of the window
		 *
		 * @note Can't find any use in the current state of Engine.
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_SetWindowSize(KarmaGuiViewport* viewport, KGVec2 size);

		/**
		 * @brief A set of instructions for rendering a viewport of KarmaGui window (called each loop for realtime Vulkan rendering). A bird's eye overview can be experienced in the wikipedia page https://github.com/ravimohan1991/KarmaEngine/wiki/Vulkan-Creative-Pipeline-(vCP)-II. Set to KarmaGuiPlatformIO::Renderer_RenderWindow in the function KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_InitPlatformInterface(). Called in the routine KarmaGuiRenderer::OnKarmaGuiLayerEnd(), via KarmaGui::RenderPlatformWindowsDefault()->platform_io.Renderer_RenderWindow, if multiviewports (KGGuiConfigFlags_ViewportsEnable) are supported.
		 *
		 * @note Engine is not supporting multiviewports in the current state.
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_RenderWindow(KarmaGuiViewport* viewport, void*);
		static void KarmaGui_ImplVulkan_RenderDrawData(KGDrawData* drawData, VkCommandBuffer commandBuffer, VkPipeline pipeline, uint32_t imageFrameIndex);
		static void KarmaGui_ImplVulkan_SwapBuffers(KarmaGuiViewport* viewport, void*);
		static void ShareVulkanContextResourcesOfMainWindow(KarmaGui_ImplVulkanH_Window* windowData, bool bCreateSyncronicity = false);
		static void ClearVulkanWindowData(KarmaGui_ImplVulkanH_Window* vulkanWindowData, bool bDestroySyncronicity = false);
		static void DestroyWindowDataFrame(KarmaGui_ImplVulkanH_ImageFrame* frame);
		static void DestroyFramesOnFlightData(KarmaGui_Vulkan_Frame_On_Flight* frameSyncronicityData);
		static void KarmaGui_ImplVulkan_CreateOrResizeWindow(KarmaGui_ImplVulkanH_Window* windowData, bool bCreateSyncronicity, bool bRecreateSwapChainAndCommandBuffers);
		static void KarmaGui_ImplVulkan_DestroyAllViewportsRenderBuffers(VkDevice device, const VkAllocationCallbacks* allocator);
		static void KarmaGui_ImplVulkan_ShivaWindowRenderBuffers(VkDevice device, KarmaGui_ImplVulkanH_WindowRenderBuffers* buffers, const VkAllocationCallbacks* allocator);
		static void KarmaGui_ImplVulkan_ShivaFrameRenderBuffers(VkDevice device, KarmaGui_ImplVulkanH_ImageFrameRenderBuffers* buffers, const VkAllocationCallbacks* allocator);

		static bool KarmaGui_ImplVulkan_CreateFontsTexture(VkCommandBuffer commandBuffer);
		static bool KarmaGui_ImplVulkan_CreateTexture(VkCommandBuffer commandBuffer, char const* fileName, const std::string& lable = "");

		/**
		 * @brief Some stuff
		 *
		 * @since Karma 1.0.0
		 */
		static bool KarmaGui_ImplVulkan_CreateDeviceObjects();
		static void KarmaGui_ImplVulkan_DestroyFontUploadObjects();
		static void KarmaGui_ImplVulkan_DestroyDeviceObjects();

		//static bool KarmaGui_ImplVulkan_Init(KarmaGui_ImplVulkan_InitInfo* info);
		static void KarmaGui_ImplVulkan_Shutdown();
		static void KarmaGui_ImplVulkan_NewFrame();

		// Register a texture (VkDescriptorSet == ImTextureID)
		// FIXME: This is experimental in the sense that we are unsure how to best design/tackle this problem, please post to 	https://github.com/ocornut/imgui/pull/914 if you have suggestions.
		static VkDescriptorSet KarmaGui_ImplVulkan_AddTexture(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);
		static void KarmaGui_ImplVulkan_InitPlatformInterface();
		static void KarmaGui_ImplVulkan_ShutdownPlatformInterface();
	};
}
