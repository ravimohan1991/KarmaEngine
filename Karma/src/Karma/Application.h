#pragma once

#include "krpch.h"

#include "Karma/Window.h"
#include "Karma/Events/ApplicationEvent.h"
#include "Karma/Events/ControllerDeviceEvent.h"
#include "Karma/LayerStack.h"
#include "Karma/ImGui/ImGuiLayer.h"
#include "Karma/Input.h"
#include "Scene.h"

namespace Karma
{
	enum class RunningPlatform
	{
		Linux = 0,
		Mac,
		Windows
	};

	class KARMA_API Application
	{
	public:
		Application();
		~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void PrepareApplicationForRun();
		void HookInputSystem(std::shared_ptr<Input> input);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() const { return *m_Window; }

		void CloseApplication();

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);
		bool OnControllerDeviceConnected(ControllerDeviceConnectedEvent& event);
		bool OnControllerDeviceDisconnected(ControllerDeviceDisconnectedEvent& event);

		// We are using raw pointers because we want necessary control over the lifetime of
		// the objects. Especially for clearing up Vulkan relevant parts.
		Window* m_Window;
		LayerStack* m_LayerStack;

		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;

		static Application* s_Instance;
		static RunningPlatform m_RPlatform;
	};

	// To be defined in the client application
	Application* CreateApplication();
}
