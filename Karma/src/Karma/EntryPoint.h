#pragma once

// Karma's entry point

// If we are on Windows
#ifdef KR_WINDOWS_PLATFORM

// CreateApplication should be implemented in client
extern Karma::Application* Karma::CreateApplication();

int main(int argc, char** argv)
{
	// TODO: add engine initialization code for various systems
	Karma::Log::Init();
	KR_CORE_WARN("Initialized log");
	KR_INFO("Hello Cowboy. Your lucky number is {0}", 7);
	
	auto app = Karma::CreateApplication();
	app->Run();
	delete app;
	
	return 0;
}

// If we are on Linux
#elif defined KR_LINUX_PLATFORM

// CreateApplication should be implemented in client
extern Karma::Application* Karma::CreateApplication();

int main(int argc, char** argv)
{
	// TODO: add engine initialization code for various systems
	Karma::Log::Init();
	KR_CORE_WARN("Initialized log");
	KR_INFO("Hello Cowboy. Your lucky number is {0}", 7);
	
	auto app = Karma::CreateApplication();
	app->Run();
	delete app;
	
	return 0;
}

#endif