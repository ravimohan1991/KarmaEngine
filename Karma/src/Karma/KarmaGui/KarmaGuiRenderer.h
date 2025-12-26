/**
 * @file KarmaGuiRenderer.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the renderer for Karma's UI
 * @version 1.0
 * @date February 19, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "imgui_impl_glfw.h"

#include "Platform/Vulkan/KarmaGuiVulkanHandler.h"
#include "Platform/OpenGL/KarmaGuiOpenGLHandler.h"

namespace Karma
{
	/**
	 * @brief A multiply inherited class for supporting both OpenGL and Vulkan API's.
	 *
	 * @note Seems like a flaw in design? Try using polymorphism appropriately.
	 * @todo This just fattens the KarmaBackendRendererUserData with disregard to I (Interface seggregation principle) of SOLID principles. Try using polymorphism by wormbo's "other way round" phrase and making OpenGL3_Data and Vulkan_Data child classes of KarmaGuiBackendRendererUserData.
	 *
	 * @since Karma 1.0.0
	 */
	struct KARMA_API KarmaGuiBackendRendererUserData : public KarmaGui_ImplOpenGL3_Data, public KarmaGui_ImplVulkan_Data
	{
		KGTextureID GetTextureIDAtIndex(uint32_t index);
		uint32_t GetTextureWidthAtIndex(uint32_t index);

		uint32_t GetTextureHeightAtIndex(uint32_t index);
	};

	/**
	 * @brief Renderer for the KarmaGui
	 * 
	 * @since Karma 1.0.0
	 */
	class KARMA_API KarmaGuiRenderer
	{
	public:
		/**
		 * @brief Sets the renderer for KarmaGui based on programmer's choice
		 *
		 * @todo Move the Vulkan part to KarmaGuiVulkanHandler for instance KarmaGui_ImplVulkan_Init. Or maybe just ponder first since I seem to have attempted that.
		 */
		static void SetUpKarmaGuiRenderer(GLFWwindow* window);
		static void OnKarmaGuiLayerDetach();
		static void OnKarmaGuiLayerBegin();
		static void OnKarmaGuiLayerEnd();

		/**
		 * @brief Acessor function for KarmaGui's renderer backend (BackendRendererUserData).
		 *
		 * @note We define our own KarmaGuiBackendRendererUserData for supporting OpenGL and Vulkan.
		 *
		 * @since Karma 1.0.0
		 */
		static KarmaGuiBackendRendererUserData* GetBackendRendererUserData();
		static void AddImageTexture(char const* fileName, const std::string& label);

		////////////////////////////// Vulkan specific functions //////////////////////////////
		/**
		 * @brief Initialize Vulkan backend renderer
		 *
		 * @todo Think about using moving the function to KarmaGuiVulkanHandler::KarmaGui_ImplVulkan_Init instead (for better keeping IDK?)
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplVulkan_Init(KarmaGui_ImplVulkan_InitInfo* initInfo);
		static void CreateDescriptorPool(VkDevice VulkanDevice);

		/**
		 * @brief Calls CleanUpVulkanAndWindowData() and does the shutting of GLFW and KarmaGui (KarmaGui::DestroyContext)
		 * 
		 * @since Karma 1.0.0
		 */
		static void GracefulVulkanShutDown();

		/**
		 * @brief Cleaning up vulkan allocated resources.
		 *
		 * @since Karma 1.0.0
		 */
		static void CleanUpVulkanAndWindowData();
		static void GiveLoopBeginControlToVulkan();
		static void GiveLoopEndControlToVulkan();

		/**
		 * @brief Commandbuffer recording for KarmaGui primitives along with additional 2D render targets in 
		 * KarmaGui_ImplVulkan_Data::Elements3DTo2D and submission to vulkan queue
		 * 
		 * @param windowData						The data required for recording the commandbuffer for KarmaGui primitives
		 * @param drawData							KarmaGui primitives, basically containing KGDrawList, total vertex and index counts
		 * 
		 * @since Karma 1.0.0
		 */
		static void FrameRender(KarmaGui_ImplVulkanH_Window* windowData, KGDrawData* drawData);

		/**
		 * @brief Issues vkQueuePresentKHR to present graphics on screen
		 * 
		 * @param windowData						The data required for the frames_in_flight tracking, swapchain presenting, and semaphore signalling
		 * 
		 * @since Karma 1.0.0
		 */
		static void FramePresent(KarmaGui_ImplVulkanH_Window* windowData);

		////////////////////////////// End of Vulkan specific functions //////////////////////////////
				
		/**
		 * @brief Creates the vulkan resources for 2D texture (render target) from 3D meshes or scene
		 * 
		 * @param scene								The 3D scene to be rendered to 2D texture
		 * @param dimensions						The dimensions of KarmaGui window in which 2D texture is displayed
		 * 
		 * @see texture id being used in KarmaGuiMesa::Draw3DModelExhibitorMesa
		 * 
		 * @since Karma 1.0.0 
		 */
		static KGTextureID Add3DSceneFor2DRendering(std::shared_ptr<Scene> scene, KGVec2 dimensions);

	private:

		static GLFWwindow* m_GLFWwindow;
		static void* m_BackendRenderer;

		// Vulkan specific members
		static VkDescriptorPool m_KarmaGuiDescriptorPool;
		static KarmaGui_ImplVulkanH_Window m_VulkanWindowData;

		static bool m_SwapChainRebuild;
	};
}
