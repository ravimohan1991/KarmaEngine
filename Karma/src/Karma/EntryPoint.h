/**
 * @file EntryPoint.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the main() routine with multiplatform support
 * @version 1.0
 * @date November 27, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once
#include "Karma.h"

// Karma's entry point

// If we are on Windows
#ifdef KR_WINDOWS_PLATFORM

// CreateApplication should be implemented in client
/**
 * @file EntryPoint.h
 * @brief Declaration of Karma::Application variable
 */
extern Karma::Application* Karma::CreateApplication();

/**
 * @file EntryPoint.h
 * @brief The C++ main() function of the Engine, for Windows.
 *
 * @param argc						argument count
 * @param argv						argument vector (argv[i] is the argument at index i)
 *
 * @return 0 on sucessful completion, else an integer (non-zero)
 * @since
 */
int main(int argc, char** argv)
{
	// TODO: add engine initialization code for various systems
	Karma::Log::Init();
	Karma::RenderCommand::Init();
	KR_INFO("Hello Cowboy. Your lucky number is {0}", 7);

	auto app = Karma::CreateApplication();

	Karma::Input::Init();

	app->PrepareApplicationForRun();
	app->Run();

	delete app;

	Karma::RenderCommand::DeInit();
	Karma::Input::DeInit();

	return 0;
}

// If we are on Linux
#elif defined KR_LINUX_PLATFORM

// CreateApplication should be implemented in client
extern Karma::Application* Karma::CreateApplication();
/**
 * @brief The C++ main() function of the Engine, for Linux.
 *
 * @param argc						argument count
 * @param argv						argument vector (argv[i] is the argument at index i)
 *
 * @return 0 on sucessful completion, else an integer (non-zero)
 * @since
 */
int main(int argc, char** argv)
{
	// TODO: add engine initialization code for various systems
	Karma::Log::Init();
	Karma::RenderCommand::Init();
	KR_INFO("Hello Cowboy. Your lucky number is {0}", 7);
	
	auto app = Karma::CreateApplication();

	Karma::Input::Init();

	app->PrepareApplicationForRun();
	app->Run();

	delete app;
	
	Karma::RenderCommand::DeInit();
	Karma::Input::DeInit();

	return 0;
}

// If we are on Mac
#elif defined KR_MAC_PLATFORM

// CreateApplication should be implemented in client
extern Karma::Application* Karma::CreateApplication();

/**
 * @brief The C++ main() function of the Engine, for MacOS.
 *
 * @param argc						argument count
 * @param argv						argument vector (argv[i] is the argument at index i)
 *
 * @return 0 on sucessful completion, else an integer (non-zero)
 * @since
 */
int main(int argc, char** argv)
{
	// TODO: add engine initialization code for various systems
	Karma::Log::Init();
	Karma::RenderCommand::Init();
	KR_INFO("Hello Cowboy. Your lucky number is {0}", 7);
	
	auto app = Karma::CreateApplication();
	
	Karma::Input::Init();

	app->PrepareApplicationForRun();
	app->Run();
	delete app;
	
	Karma::RenderCommand::DeInit();
	Karma::Input::DeInit();
	
	return 0;
}

#endif
