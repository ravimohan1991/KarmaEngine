#pragma once

#include "krpch.h"

#include "Karma/Layer.h"
#include "Karma/Events/KeyEvent.h"
#include "Karma/Events/MouseEvent.h"
#include "Karma/Events/ApplicationEvent.h"
#include "Karma/Window.h"
#include "vulkan/vulkan_core.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "ImGuiVulkanHandler.h"

namespace Karma
{
	class KARMA_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(Window* relevantWindow);
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		virtual void OnUpdate(float deltaTime) override;

		// Only for ImGui layer
		void Begin();
		void End();

		virtual void OnEvent(Event& event) override;

		void SetMenubarCallback(const std::function<void()>& menubarCallback) { m_MenubarCallback = menubarCallback; }

		// Vulkan relevant functions
		void CreateDescriptorPool();
		void GiveLoopBeginControlToVulkan();
		void FrameRender(ImGui_KarmaImplVulkanH_Window* windowData, ImDrawData* drawData);
		void FramePresent(ImGui_KarmaImplVulkanH_Window* windowData);
		void GiveLoopEndControlToVulkan();
		void GracefulVulkanShutDown();
		void CleanUpVulkanAndWindowData();
		
		/*
		void GatherVulkanWindowData(ImGui_KarmaImplVulkanH_Window* vulkanWindowData, int width, int height);
		void GiveLoopBeginControlToVulkan();
		void GiveLoopEndControlToVulkan();
		void FrameRender(ImGui_KarmaImplVulkanH_Window* windowData, ImDrawData* draw_data);
		void FramePresent(ImGui_KarmaImplVulkanH_Window* windowData);
		void CleanUpVulkanAndWindowData();
		void GracefulVulkanShutDown();
		void ShareVulkanContextOfMainWindow(ImGui_KarmaImplVulkanH_Window* windowData, bool bCreateSyncronicity = false);
		void ClearVulkanWindowData(ImGui_KarmaImplVulkanH_Window* vulkanWindowData, bool bDestroySyncronicity = false);
		void DestroyWindowDataFrame(ImGui_KarmaImplVulkanH_ImageFrame* frame);
		void DestroyFramesOnFlightData(ImGui_Vulkan_Frame_On_Flight* frameSyncronicityData);
		*/

	private:
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool OnMouseMovedEvent(MouseMovedEvent& e);
		bool OnMouseScrollEvent(MouseScrolledEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyReleasedEvent(KeyReleasedEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);

	private:
		float m_Time = 0.0f;
		//int MAX_FRAMES_IN_FLIGHT = 0;
		//size_t m_CurrentFrame = 0;
		//uint32_t imageIndex = 0;

		Window* m_AssociatedWindow;

		// Vulkan specific members
		VkDescriptorPool m_ImGuiDescriptorPool;

		//VkDevice m_Device;
		//VkInstance m_Instance;
		
		
		bool m_SwapChainRebuild = true;

		// Seems like main window Vulkan data associated to m_AssociatedWindow
		ImGui_KarmaImplVulkanH_Window m_VulkanWindowData;

		std::function<void()> m_MenubarCallback;
	};
}
