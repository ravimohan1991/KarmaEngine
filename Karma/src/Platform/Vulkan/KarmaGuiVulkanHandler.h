// Major and heavy modifications of the original Dear ImGUI's take on Vulkan implementation
// https://github.com/ravimohan1991/imgui/blob/9068fd1afbaba56247aacd452bbadebb286800c7/backends/imgui_impl_vulkan.h

#pragma once

#include "krpch.h"

#include "KarmaGui.h"
#include "KarmaGuiInternal.h"
#include "Karma/Renderer/Scene.h"
#include <vulkan/vulkan.h>

namespace Karma
{
	// Initialization data, for ImGui_ImplVulkan_Init()
	// [Please zero-clear before use!]
	struct KarmaGui_ImplVulkan_InitInfo
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
	struct KarmaGui_Vulkan_Frame_On_Flight
	{
		VkFence             Fence;
		VkSemaphore         ImageAcquiredSemaphore;
		VkSemaphore         RenderCompleteSemaphore;
		VkCommandBuffer     CommandBuffer;
	};

	// Helper structure to hold the data needed by one rendering ImageFrame (different from the FRAME_IN_FLIGHT frame!)
	// (Used by multi-viewport features. We gonna just use this.)
	// [Please zero-clear before use!]
	struct KarmaGui_ImplVulkanH_ImageFrame
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
	struct KarmaGui_ImplVulkanH_Window
	{
		int                 Width;
		int                 Height;
		VkSwapchainKHR      Swapchain;
		VkCommandPool       CommandPool;
		VkSurfaceKHR        Surface;
		VkSurfaceFormatKHR  SurfaceFormat;
		VkPresentModeKHR    PresentMode;
		VkRenderPass        RenderPass;
		VkPipeline          Pipeline;               // Caution: The window pipeline may use a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo. Need to investigate why.
		bool                ClearEnable;
		VkClearValue        ClearValue;
		uint32_t            ImageFrameIndex;             // Number of the image (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count) to be addressed for frame (each loop iteration) rendering logic.
		uint32_t            TotalImageCount;             // Total Number of the images supported by swapchain
		uint32_t            SemaphoreIndex;         // Current set of swapchain wait semaphores and command buffers we're using (0 <= SemaphoreIndex < MAX_FRAMES_IN_FLIGHT)
		uint32_t                 MAX_FRAMES_IN_FLIGHT;
		KarmaGui_ImplVulkanH_ImageFrame* ImageFrames; // Cowboy's Note: Not the regular frame sense. Just a container for buffers and all those sizes depending on  VulkanHolder::GetVulkanContext()->GetSwapChainImages().size();

		// Cowboy's Modification
		KarmaGui_Vulkan_Frame_On_Flight* FramesOnFlight; // The syncronicity data for in-flight frames
		VkRect2D                            RenderArea;

		KarmaGui_ImplVulkanH_Window()
		{
			memset((void*)this, 0, sizeof(*this));
			PresentMode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
			ClearEnable = true;
		}
	};

	// Reusable buffers used for rendering 1 current in-flight ImageFrame, for ImGui_ImplVulkan_RenderDrawData()
	// Seems like data structure with single instantiation for each of the FrameIndex
	// [Please zero-clear before use!]
	struct KarmaGui_ImplVulkanH_ImageFrameRenderBuffers
	{
		VkDeviceMemory      VertexBufferMemory;
		VkDeviceMemory      IndexBufferMemory;
		VkDeviceSize        VertexBufferSize;
		VkDeviceSize        IndexBufferSize;
		VkBuffer            VertexBuffer;
		VkBuffer            IndexBuffer;

	public:
		KarmaGui_ImplVulkanH_ImageFrameRenderBuffers()
		{
			VertexBufferSize = IndexBufferSize = 0;
			IndexBuffer = VertexBuffer = VK_NULL_HANDLE;
			IndexBufferMemory = VertexBufferMemory = VK_NULL_HANDLE;
		}
	};

	// Each viewport will hold 1 ImGui_ImplVulkanH_WindowRenderBuffers
	// [Please zero-clear before use!]
	struct KarmaGui_ImplVulkanH_WindowRenderBuffers
	{
		uint32_t            Index;
		uint32_t            Count;
		KarmaGui_ImplVulkanH_ImageFrameRenderBuffers* FrameRenderBuffers;
	};

	// For multi-viewport support:
	// Helper structure we store in the void* RenderUserData field of each ImGuiViewport to easily retrieve our backend data.
	struct KarmaGui_ImplVulkan_ViewportData
	{
		bool                                    WindowOwned;
		KarmaGui_ImplVulkanH_Window                Window;             // Used by secondary viewports only
		KarmaGui_ImplVulkanH_WindowRenderBuffers   RenderBuffers;      // Used by all viewports
		//ImGui_ImplVulkanH_Window*               ImGUILayerWindowData;

		KarmaGui_ImplVulkan_ViewportData()
		{
			WindowOwned = false;
			memset(&RenderBuffers, 0, sizeof(RenderBuffers));
		}
		~KarmaGui_ImplVulkan_ViewportData() { }
	};

	// Vulkan Image data
	struct KarmaGui_ImplVulkan_Image_TextureData
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

		KarmaGui_ImplVulkan_Image_TextureData()
		{
			TextureLable = "NO LABLE YET";
		}
	};

	// Vulkan data
	// Seems for single ImGUI context (which can have multiple window)
	struct KarmaGui_ImplVulkan_Data
	{
		KarmaGui_ImplVulkan_InitInfo   VulkanInitInfo;
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
		std::vector<KarmaGui_ImplVulkan_Image_TextureData*>         vulkanMesaDecalDataList;

		// Render buffers for main window
		KarmaGui_ImplVulkanH_WindowRenderBuffers MainWindowRenderBuffers;
		
		// heh, not working with initializers in MSVC. Need a blog post!
		
		/*
		ImGui_ImplVulkan_Data()
		{
			memset((void*)this, 0, sizeof(*this));
			BufferMemoryAlignment = 256;
		}*/
	};

	class KARMA_API KarmaGuiVulkanHandler
	{
	public:
		/*
		inline static KarmaGui_ImplVulkan_Data* KarmaGui_ImplVulkan_GetBackendData()
		{
			return KarmaGui::GetCurrentContext() ? (KarmaGui_ImplVulkan_Data*)KarmaGui::GetIO().BackendRendererUserData : nullptr;
		}*/
		// GetIO should fetche the configuration settings and whatnot, which in this case is the struct KarmaGui_ImplVulkan_Data

		static uint32_t KarmaGui_ImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits);
		static void CreateOrResizeBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize& pBufferSize, size_t newSize, VkBufferUsageFlagBits usage);
		static void KarmaGui_ImplVulkan_SetupRenderState(KGDrawData* drawData, VkPipeline pipeline, VkCommandBuffer commandBuffer, KarmaGui_ImplVulkanH_ImageFrameRenderBuffers* remderingBufferData, int width, int height);
		
		// Experimental
		static void KarmaGui_ImplVulkan_SetupRenderStateFor3DRendering(Scene* sceneToDraw, VkCommandBuffer commandBuffer, KGDrawData* drawData);
		
		static void KarmaGui_ImplVulkan_CreateShaderModules(VkDevice device, const VkAllocationCallbacks* allocator);
		static void KarmaGui_ImplVulkan_CreateFontSampler(VkDevice device, const VkAllocationCallbacks* allocator);
		static void KarmaGui_ImplVulkan_CreateDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks* allocator);
		static void KarmaGui_ImplVulkan_CreatePipelineLayout(VkDevice device, const VkAllocationCallbacks* allocator);
		static void KarmaGui_ImplVulkan_CreatePipeline(VkDevice device, const VkAllocationCallbacks* allocator, VkPipelineCache pipelineCache, VkRenderPass renderPass, VkSampleCountFlagBits MSAASamples, VkPipeline* pipeline, uint32_t subpass);
		static void KarmaGui_ImplVulkan_CreateWindow(KarmaGuiViewport* viewport);

		// Overload
		static void KarmaGui_ImplVulkan_DestroyWindow(KarmaGuiViewport* viewport);
		static void KarmaGui_ImplVulkan_DestroyWindow(KarmaGui_ImplVulkanH_Window* windowData);

		static void KarmaGui_ImplVulkan_SetWindowSize(KarmaGuiViewport* viewport, KGVec2 size);
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

		static bool KarmaGui_ImplVulkan_CreateDeviceObjects();
		static void KarmaGui_ImplVulkan_DestroyFontUploadObjects();
		static void KarmaGui_ImplVulkan_DestroyDeviceObjects();

		static bool KarmaGui_ImplVulkan_Init(KarmaGui_ImplVulkan_InitInfo* info);
		static void KarmaGui_ImplVulkan_Shutdown();
		static void KarmaGui_ImplVulkan_NewFrame();

		// Register a texture (VkDescriptorSet == ImTextureID)
		// FIXME: This is experimental in the sense that we are unsure how to best design/tackle this problem, please post to 	https://github.com/ocornut/imgui/pull/914 if you have suggestions.
		static VkDescriptorSet KarmaGui_ImplVulkan_AddTexture(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);
		static void KarmaGui_ImplVulkan_InitPlatformInterface();
		static void KarmaGui_ImplVulkan_ShutdownPlatformInterface();
	};
}
