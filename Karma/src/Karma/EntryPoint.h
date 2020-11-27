#pragma once

// Karma's entry point


// If we are on windows
#ifdef KR_WINDOWS_PLATFORM

// CreateApplication should be implemented in client
extern Karma::Application* Karma::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Karma::CreateApplication();
	app->Run();
	delete app;
	
	return 0;
}

#endif