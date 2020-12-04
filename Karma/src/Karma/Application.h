#pragma once

#include "Karma/Core.h"
#include "Karma/Window.h"
#include "Karma/Events/ApplicationEvent.h"
#include "Karma/LayerStack.h"

#include <memory>

namespace Karma
{
	class KARMA_API Application
	{
	public:
		Application();
		~Application();
		
		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

	private:
		bool OnWindowClose(WindowCloseEvent& event);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;// Created on stack. For entire lifetime of the program (singleton?)
	};

	// To be defined in the client application
	Application* CreateApplication();
}
