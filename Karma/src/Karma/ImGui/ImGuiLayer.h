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
		ImGuiLayer(std::shared_ptr<Window> relevantWindow);
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		virtual void OnUpdate(float deltaTime) override;
		
		// Only for ImGui layer
		void Begin();
		void End();
		
		virtual void OnEvent(Event& event) override;
		
		//static void ImguiVulkanRenderFrame(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
		void AllocateImGuiCommandBuffers();
	
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
		
		std::shared_ptr<Window> m_AssociatedWindow;
		VkDescriptorPool m_ImguiPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}
