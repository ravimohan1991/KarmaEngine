#pragma once

#include "krpch.h"

#include "imgui_impl_glfw.h"
#include "Platform/Vulkan/KarmaGuiVulkanHandler.h"
#include "Platform/OpenGL/KarmaGuiOpenGLHandler.h"

namespace Karma
{
	class KARMA_API KarmaGuiRenderer
	{
	public:
		static void SetUpKarmaGuiRenderer(GLFWwindow* window);
		static void OnKarmaGuiLayerDetach();
		static void OnKarmaGuiLayerBegin();
		static void OnKarmaGuiLayerEnd();
		static void* GetBackendRendererUserData();

		// Vulkan specific functions
		static void CreateDescriptorPool();
		static void GracefulVulkanShutDown();
		static void CleanUpVulkanAndWindowData();
		static void GiveLoopBeginControlToVulkan();
		static void GiveLoopEndControlToVulkan();
		static void FrameRender(KarmaGui_ImplVulkanH_Window* windowData, KGDrawData* drawData);
		static void FramePresent(KarmaGui_ImplVulkanH_Window* windowData);

	private:

		static GLFWwindow* m_GLFWwindow;
		static void* m_BackendRenderer;

		// Vulkan specific members
		static VkDescriptorPool m_KarmaGuiDescriptorPool;
		static KarmaGui_ImplVulkanH_Window m_VulkanWindowData;

		static bool m_SwapChainRebuild;
	};
}