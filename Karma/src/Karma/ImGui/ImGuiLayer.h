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
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

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
		VkCommandBuffer AllocateImGuiCommandBuffers(bool bBegin);
		void CreateDescriptorPool();
		void GatherVulkanWindowData(ImGui_ImplVulkanH_Window* vulkanWindowData, VkSurfaceKHR surface, int width, int height);
		void GiveLoopBeginControlToVulkan();
		void GiveLoopEndControlToVulkan();
		void FrameRender(ImGui_ImplVulkanH_Window* windowData, ImDrawData* draw_data);
		void FramePresent(ImGui_ImplVulkanH_Window* windowData);
		void CleanUpVulkanAndWindowData();
		void GracefulVulkanShutDown();
	
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
		uint32_t m_CommandBuffersSize = 1;
		
		Window* m_AssociatedWindow;
		
		// Vulkan specific members
		VkDescriptorPool m_ImGuiDescriptorPool;
		VkDevice m_Device;
		bool m_SwapChainRebuild;
		uint32_t m_CurrentFrameIndex;
		ImGui_ImplVulkanH_Window m_VulkanWindowData;
		
		uint32_t m_MinImageCount = 3;
		
		// Per-frame-in-flight
		std::vector<std::vector<VkCommandBuffer>> m_AllocatedCommandBuffers;
		std::vector<std::vector<std::function<void()>>> m_ResourceFreeQueue;
		
		std::function<void()> m_MenubarCallback;
	};
}
