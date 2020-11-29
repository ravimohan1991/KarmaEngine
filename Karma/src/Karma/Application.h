#pragma once

#include "Core.h"
#include "Window.h"

namespace Karma
{
	class KARMA_API Application
	{
	public:
		Application();
		~Application();
		
		void Run();

	private:
		std::unique_ptr<Window> m_Window;
	};

	// To be defined in the client application
	Application* CreateApplication();
}
