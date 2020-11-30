#pragma once

#include "Core.h"
#include "Window.h"
#include "Karma/Events/ApplicationEvent.h"

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

	private:
		bool OnWindowClose(WindowCloseEvent& event);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// To be defined in the client application
	Application* CreateApplication();
}
