// Major and heavy modifications of the original Dear ImGUI's take on Vulkan implementation
// https://github.com/ravimohan1991/imgui/blob/9068fd1afbaba56247aacd452bbadebb286800c7/backends/imgui_impl_vulkan.h

#pragma once

#include "krpch.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Karma/Renderer/Scene.h"
#include <vulkan/vulkan.h>

namespace Karma
{
	// Initialization data, for ImGui_KarmaImplVulkan_Init()
	// [Please zero-clear before use!]
	struct ImGui_KarmaImplVulkan_InitInfo
	{
		VkInstance                      Instance;
		VkPhysicalDevice                PhysicalDevice;
		VkDevice                        Device;
		uint32_t                        QueueFamily;
		VkQueue                         Queue;
		VkPipelineCache                 PipelineCache;
		VkDescriptorPool                DescriptorPool;
		VkRenderPass                    RenderPass; // Very experimental here
		uint32_t                        Subpass;
		uint32_t                        MinImageCount;          // >= 2
		uint32_t                        ImageCount;             // >= MinImageCount
		VkSampleCountFlagBits           MSAASamples;            // >= VK_SAMPLE_COUNT_1_BIT (0 -> default to VK_SAMPLE_COUNT_1_BIT)
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
	// Your engine/app will likely _already_ have code to setup all that stuff (swap chain, render pass, frame buffers, etc.).
	// You may read this code to learn about Vulkan, but it is recommended you use you own custom tailored code to do equivalent work.
	// (The ImGui_KarmaImplVulkanH_XXX functions do not interact with any of the state used by the regular ImGui_KarmaImplVulkan_XXX 	functions)
	//-------------------------------------------------------------------------

	// Cowboy's confusion clarification concept!
	// It seems the ImGUI author(s) have mixed and/or confused notion of
	// ImageCount, which decides the number of SwapChainImages, framebuffer, and so on (contained within
	// ImGui_KarmaImplVulkanH_ImageFrame structure).
	// (https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Retrieving-the-swap-chain-images)
	// and MAX_FRAMES_IN_FLIGHT, which is representative of (linearly proportional to or indicative of) number of commandbuffer recordings on CPU that may happen whilst the rendering is being done on GPU. That should determine the semaphore, fence, and commandbuffer size.
	// https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Frames_in_flight
	// The argument is elicited by the comment line https://github.com/ravimohan1991/imgui/blob/e4967701b67edd491e884632f239ab1f38867d86/backends/imgui_impl_vulkan.h#L144
	struct ImGui_Vulkan_Frame_On_Flight
	{
		VkFence             Fence;
		VkSemaphore         ImageAcquiredSemaphore;
		VkSemaphore         RenderCompleteSemaphore;
		VkCommandBuffer     CommandBuffer;
	};

	// Helper structure to hold the data needed by one rendering ImageFrame (different from the FRAME_IN_FLIGHT frame!)
	// (Used by multi-viewport features. We gonna just use this.)
	// [Please zero-clear before use!]
	struct ImGui_KarmaImplVulkanH_ImageFrame
	{
		//VkCommandPool       CommandPool;
		//VkCommandBuffer     CommandBuffer;
		VkImage             Backbuffer;                   // VulkanContext m_swapChainImages equivalent
		VkImageView         BackbufferView;               // VulkanContext m_swapChainImageViews equivalent
		VkFramebuffer       Framebuffer;
	};

	// Helper structure to hold the data needed by one rendering context into one OS window
	// Seems like the appropriate inherited instantiation of our Window class (written in Window.h),
	// created in Application (https://github.com/ravimohan1991/KarmaEngine/blob/main/Karma/src/Karma/Application.cpp)
	// (Used by multi-viewport features. We gonna just use this.)
	struct ImGui_KarmaImplVulkanH_Window
	{
		int                 Width;
		int                 Height;
		VkSwapchainKHR      Swapchain;
		VkCommandPool       CommandPool;
		VkSurfaceKHR        Surface;
		VkSurfaceFormatKHR  SurfaceFormat;
		VkPresentModeKHR    PresentMode;
		VkRenderPass        RenderPass;
		VkPipeline          Pipeline;               // Caution: The window pipeline may use a different VkRenderPass than the one passed in ImGui_KarmaImplVulkan_InitInfo. Need to investigate why.
		bool                ClearEnable;
		VkClearValue        ClearValue;
		uint32_t            ImageFrameIndex;             // Number of the image (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count) to be addressed for frame (each loop iteration) rendering logic.
		uint32_t            TotalImageCount;             // Total Number of the images supported by swapchain
		uint32_t            SemaphoreIndex;         // Current set of swapchain wait semaphores and command buffers we're using (0 <= SemaphoreIndex < MAX_FRAMES_IN_FLIGHT)
		uint32_t                 MAX_FRAMES_IN_FLIGHT;
		ImGui_KarmaImplVulkanH_ImageFrame* ImageFrames; // Cowboy's Note: Not the regular frame sense. Just a container for buffers and all those sizes depending on  VulkanHolder::GetVulkanContext()->GetSwapChainImages().size();

		// Cowboy's Modification
		ImGui_Vulkan_Frame_On_Flight* FramesOnFlight; // The syncronicity data for in-flight frames
		VkRect2D                            RenderArea;

		ImGui_KarmaImplVulkanH_Window()
		{
			memset((void*)this, 0, sizeof(*this));
			PresentMode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
			ClearEnable = true;
		}
	};

	// Reusable buffers used for rendering 1 current in-flight ImageFrame, for ImGui_KarmaImplVulkan_RenderDrawData()
	// Seems like data structure with single instantiation for each of the FrameIndex
	// [Please zero-clear before use!]
	struct ImGui_KarmaImplVulkanH_ImageFrameRenderBuffers
	{
		VkDeviceMemory      VertexBufferMemory;
		VkDeviceMemory      IndexBufferMemory;
		VkDeviceSize        VertexBufferSize;
		VkDeviceSize        IndexBufferSize;
		VkBuffer            VertexBuffer;
		VkBuffer            IndexBuffer;

	public:
		ImGui_KarmaImplVulkanH_ImageFrameRenderBuffers()
		{
			VertexBufferSize = IndexBufferSize = 0;
			IndexBuffer = VertexBuffer = VK_NULL_HANDLE;
			IndexBufferMemory = VertexBufferMemory = VK_NULL_HANDLE;
		}
	};

	// Each viewport will hold 1 ImGui_KarmaImplVulkanH_WindowRenderBuffers
	// [Please zero-clear before use!]
	struct ImGui_KarmaImplVulkanH_WindowRenderBuffers
	{
		uint32_t            Index;
		uint32_t            Count;
		ImGui_KarmaImplVulkanH_ImageFrameRenderBuffers* FrameRenderBuffers;
	};

	// For multi-viewport support:
	// Helper structure we store in the void* RenderUserData field of each ImGuiViewport to easily retrieve our backend data.
	struct ImGui_KarmaImplVulkan_ViewportData
	{
		bool                                    WindowOwned;
		ImGui_KarmaImplVulkanH_Window                Window;             // Used by secondary viewports only
		ImGui_KarmaImplVulkanH_WindowRenderBuffers   RenderBuffers;      // Used by all viewports
		//ImGui_KarmaImplVulkanH_Window*               ImGUILayerWindowData;

		ImGui_KarmaImplVulkan_ViewportData()
		{
			WindowOwned = false;
			memset(&RenderBuffers, 0, sizeof(RenderBuffers));
		}
		~ImGui_KarmaImplVulkan_ViewportData() { }
	};

	// Vulkan Image data
	struct ImGui_KarmaImplVulkan_Image_TextureData
	{
		std::string                 TextureLable;
		uint32_t                    width;
		uint32_t                    height;
		uint32_t                    channels;
		size_t                      size;

		VkSampler                   TextureSampler;
		VkDeviceMemory              TextureMemory;
		VkImage                     TextureImage;
		VkImageView                 TextureView;
		VkDescriptorSet             TextureDescriptorSet;
		VkDeviceMemory              UploadBufferMemory;
		VkBuffer                    UploadBuffer;
	};

	// Vulkan data
	// Seems for single ImGUI context (which can have multiple window)
	struct ImGui_KarmaImplVulkan_Data
	{
		ImGui_KarmaImplVulkan_InitInfo   VulkanInitInfo;
		VkRenderPass                RenderPass;
		VkDeviceSize                BufferMemoryAlignment;
		VkPipelineCreateFlags       PipelineCreateFlags;
		VkDescriptorSetLayout       DescriptorSetLayout;
		VkPipelineLayout            PipelineLayout;
		VkPipeline                  Pipeline;
		uint32_t                    Subpass;
		VkShaderModule              ShaderModuleVert;
		VkShaderModule              ShaderModuleFrag;

		// Font data
		VkSampler                   FontSampler;
		VkDeviceMemory              FontMemory;
		VkImage                     FontImage;
		VkImageView                 FontView;
		VkDescriptorSet             FontDescriptorSet;
		VkDeviceMemory              UploadBufferMemory;
		VkBuffer                    UploadBuffer;

		// Image data
		std::vector<ImGui_KarmaImplVulkan_Image_TextureData*>         mesaDecalDataList;

		// Render buffers for main window
		ImGui_KarmaImplVulkanH_WindowRenderBuffers MainWindowRenderBuffers;

		ImGui_KarmaImplVulkan_Data()
		{
			memset((void*)this, 0, sizeof(*this));
			BufferMemoryAlignment = 256;
		}
	};

	class KARMA_API ImGuiVulkanHandler
	{
	public:
		// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
		// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) 	instead of multiple Dear ImGui contexts.
		// FIXME: multi-context support is not tested and probably dysfunctional in this backend.
		// Allow me to make an attempt, good programmer! ~ The_Cowboy
		inline static ImGui_KarmaImplVulkan_Data* ImGui_KarmaImplVulkan_GetBackendData()
		{
			return ImGui::GetCurrentContext() ? (ImGui_KarmaImplVulkan_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
		}
		// GetIO should fetche the configuration settings and whatnot, which in this case is the struct ImGui_KarmaImplVulkan_Data

		static uint32_t ImGui_KarmaImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits);
		static void CreateOrResizeBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize& pBufferSize, size_t newSize, VkBufferUsageFlagBits usage);
		static void ImGui_KarmaImplVulkan_SetupRenderState(ImDrawData* drawData, VkPipeline pipeline, VkCommandBuffer commandBuffer, ImGui_KarmaImplVulkanH_ImageFrameRenderBuffers* remderingBufferData, int width, int height);
		
		// Experimental
		static void ImGui_KarmaImplVulkan_SetupRenderStateFor3DRendering(Scene* sceneToDraw, VkCommandBuffer commandBuffer, ImDrawData* drawData);
		
		static void ImGui_KarmaImplVulkan_CreateShaderModules(VkDevice device, const VkAllocationCallbacks* allocator);
		static void ImGui_KarmaImplVulkan_CreateFontSampler(VkDevice device, const VkAllocationCallbacks* allocator);
		static void ImGui_KarmaImplVulkan_CreateDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks* allocator);
		static void ImGui_KarmaImplVulkan_CreatePipelineLayout(VkDevice device, const VkAllocationCallbacks* allocator);
		static void ImGui_KarmaImplVulkan_CreatePipeline(VkDevice device, const VkAllocationCallbacks* allocator, VkPipelineCache pipelineCache, VkRenderPass renderPass, VkSampleCountFlagBits MSAASamples, VkPipeline* pipeline, uint32_t subpass);
		static void ImGui_KarmaImplVulkan_CreateWindow(ImGuiViewport* viewport);

		// Overload
		static void ImGui_KarmaImplVulkan_DestroyWindow(ImGuiViewport* viewport);
		static void ImGui_KarmaImplVulkan_DestroyWindow(ImGui_KarmaImplVulkanH_Window* windowData);

		static void ImGui_KarmaImplVulkan_SetWindowSize(ImGuiViewport* viewport, ImVec2 size);
		static void ImGui_KarmaImplVulkan_RenderWindow(ImGuiViewport* viewport, void*);
		static void ImGui_KarmaImplVulkan_RenderDrawData(ImDrawData* drawData, VkCommandBuffer commandBuffer, VkPipeline pipeline, uint32_t imageFrameIndex);
		static void ImGui_KarmaImplVulkan_SwapBuffers(ImGuiViewport* viewport, void*);
		static void ShareVulkanContextResourcesOfMainWindow(ImGui_KarmaImplVulkanH_Window* windowData, bool bCreateSyncronicity = false);
		static void ClearVulkanWindowData(ImGui_KarmaImplVulkanH_Window* vulkanWindowData, bool bDestroySyncronicity = false);
		static void DestroyWindowDataFrame(ImGui_KarmaImplVulkanH_ImageFrame* frame);
		static void DestroyFramesOnFlightData(ImGui_Vulkan_Frame_On_Flight* frameSyncronicityData);
		static void ImGui_KarmaImplVulkan_CreateOrResizeWindow(ImGui_KarmaImplVulkanH_Window* windowData, bool bCreateSyncronicity, bool bRecreateSwapChainAndCommandBuffers);
		static void ImGui_KarmaImplVulkan_DestroyAllViewportsRenderBuffers(VkDevice device, const VkAllocationCallbacks* allocator);
		static void ImGui_KarmaImplVulkan_ShivaWindowRenderBuffers(VkDevice device, ImGui_KarmaImplVulkanH_WindowRenderBuffers* buffers, const VkAllocationCallbacks* allocator);
		static void ImGui_KarmaImplVulkan_ShivaFrameRenderBuffers(VkDevice device, ImGui_KarmaImplVulkanH_ImageFrameRenderBuffers* buffers, const VkAllocationCallbacks* allocator);

		static bool ImGui_KarmaImplVulkan_CreateFontsTexture(VkCommandBuffer commandBuffer);
		static bool ImGui_KarmaImplVulkan_CreateTexture(VkCommandBuffer commandBuffer, char const* fileName);

		static bool ImGui_KarmaImplVulkan_CreateDeviceObjects();
		static void ImGui_KarmaImplVulkan_DestroyFontUploadObjects();
		static void ImGui_KarmaImplVulkan_DestroyDeviceObjects();

		static bool ImGui_KarmaImplVulkan_Init(ImGui_KarmaImplVulkan_InitInfo* info);
		static void ImGui_KarmaImplVulkan_Shutdown();
		static void ImGui_KarmaImplVulkan_NewFrame();

		// Register a texture (VkDescriptorSet == ImTextureID)
		// FIXME: This is experimental in the sense that we are unsure how to best design/tackle this problem, please post to 	https://github.com/ocornut/imgui/pull/914 if you have suggestions.
		static VkDescriptorSet ImGui_KarmaImplVulkan_AddTexture(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);
		static void ImGui_KarmaImplVulkan_InitPlatformInterface();
		static void ImGui_KarmaImplVulkan_ShutdownPlatformInterface();
	};
}
