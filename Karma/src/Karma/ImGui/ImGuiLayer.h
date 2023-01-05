#pragma once

#include "krpch.h"

#include "Layer.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/ApplicationEvent.h"
#include "Window.h"
#include "ImGuiVulkanHandler.h"
#include "ImGuiOpenGLHandler.h"
#include "Scene.h"

namespace Karma
{
	class KARMA_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(Window* relevantWindow);
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void ImGuiRender(float deltaTime) override;
		virtual void OnUpdate(float deltaTime) override;

		// Only for ImGui layer
		void Begin();
		void End();

		virtual void OnEvent(Event& event) override;

		// Vulkan relevant functions
		void CreateDescriptorPool();
		void GiveLoopBeginControlToVulkan();
		void FrameRender(ImGui_KarmaImplVulkanH_Window* windowData, ImDrawData* drawData);
		void FramePresent(ImGui_KarmaImplVulkanH_Window* windowData);
		void GiveLoopEndControlToVulkan();
		void GracefulVulkanShutDown();
		void CleanUpVulkanAndWindowData();

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

		Window* m_AssociatedWindow;

		// Vulkan specific members
		VkDescriptorPool m_ImGuiDescriptorPool;

		bool m_SwapChainRebuild = true;

		// Seems like main window Vulkan data associated to m_AssociatedWindow
		ImGui_KarmaImplVulkanH_Window m_VulkanWindowData;
	};
}
