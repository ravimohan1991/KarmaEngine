#pragma once

#include "krpch.h"

#include "imgui_impl_glfw.h"
#include "Platform/Vulkan/KarmaGuiVulkanHandler.h"
#include "Platform/OpenGL/KarmaGuiOpenGLHandler.h"

namespace Karma
{
	struct KARMA_API KarmaGuiBackendRendererUserData : public KarmaGui_ImplOpenGL3_Data, public KarmaGui_ImplVulkan_Data
	{
		KGTextureID GetTextureIDAtIndex(uint32_t index);
		uint32_t GetTextureWidthAtIndex(uint32_t index);

		uint32_t GetTextureHeightAtIndex(uint32_t index);
	};

	class KARMA_API KarmaGuiRenderer
	{
	public:
		static void SetUpKarmaGuiRenderer(GLFWwindow* window);
		static void OnKarmaGuiLayerDetach();
		static void OnKarmaGuiLayerBegin();
		static void OnKarmaGuiLayerEnd();
		static KarmaGuiBackendRendererUserData* GetBackendRendererUserData();
		static void AddImageTexture(char const* fileName, const std::string& label);

		// Vulkan specific functions
		static void KarmaGui_ImplVulkan_Init(KarmaGui_ImplVulkan_InitInfo* initInfo);
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
