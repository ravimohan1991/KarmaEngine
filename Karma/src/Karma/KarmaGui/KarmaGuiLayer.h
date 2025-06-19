/**
 * @file KarmaGuiLayer.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class KarmaGuiLayer, a Layer between Engine and KarmaGui.
 * @version 1.0
 * @date December 7, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "krpch.h"

#include "Layer.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/ApplicationEvent.h"
#include "Window.h"
#include "Platform/Vulkan/KarmaGuiVulkanHandler.h"
#include "Platform/OpenGL/KarmaGuiOpenGLHandler.h"
#include "Scene.h"

namespace Karma
{
	class KARMA_API KarmaGuiLayer : public Layer
	{
	public:
		KarmaGuiLayer(Window* relevantWindow);
		~KarmaGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void KarmaGuiRender(float deltaTime) override;
		virtual void OnUpdate(float deltaTime) override;

		// Only for KarmaGui layer
		void Begin();
		void End();

		virtual void OnEvent(Event& event) override;

		// Vulkan relevant functions
		void CreateDescriptorPool();
		void GiveLoopBeginControlToVulkan();
		void FrameRender(KarmaGui_ImplVulkanH_Window* windowData, KGDrawData* drawData);
		void FramePresent(KarmaGui_ImplVulkanH_Window* windowData);
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
		//VkDescriptorPool m_ImGuiDescriptorPool;

		//bool m_SwapChainRebuild = true;

		// Seems like main window Vulkan data associated to m_AssociatedWindow
		//KarmaGui_ImplVulkanH_Window m_VulkanWindowData;
	};
}
