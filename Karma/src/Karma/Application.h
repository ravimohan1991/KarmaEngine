#pragma once

#include "Core.h"

namespace Karma
{
	class KARMA_API Application
	{
	public:
		Application();
		~Application();
		
		void Run();
	};

	// To be defined in the client application
	Application* CreateApplication();
}
