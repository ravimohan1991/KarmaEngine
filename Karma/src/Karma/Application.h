#pragma once

#include "krpch.h"

#include "Karma/Window.h"
#include "Karma/Events/ApplicationEvent.h"
#include "Karma/Events/ControllerDeviceEvent.h"
#include "Karma/LayerStack.h"
#include "Karma/ImGui/ImGuiLayer.h"
#include "Karma/Input.h"

#include <memory>

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

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);
		bool OnControllerDeviceConnected(ControllerDeviceConnectedEvent& event);
		bool OnControllerDeviceDisconnected(ControllerDeviceDisconnectedEvent& event);

		std::shared_ptr<Window> m_Window;
		//ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;// Created on stack. For entire lifetime of the program (singleton?)

		static Application* s_Instance;
		static RunningPlatform m_RPlatform;
	};

	// To be defined in the client application
	Application* CreateApplication();
}
