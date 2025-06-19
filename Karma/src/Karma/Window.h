/**
 * @file Window.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the Window class.
 * @version 1.0
 * @date November 29, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Core.h"
#include "Karma/Events/Event.h"

#include "Karma/Events/ApplicationEvent.h"

namespace Karma
{
	/**
	 * @brief The "tangible" properties of a window
	 */
	struct WindowProps
	{
		/**
		 * @brief The title of the window
		 *
		 * @since Karma 1.0.0
		 */
		std::string Title;
		/**
		 * @brief The width of the window
		 *
		 * @since Karma 1.0.0
		 */
		unsigned int Width;
		/**
		 * @brief The height of the window
		 *
		 * @since Karma 1.0.0
		 */
		unsigned int Height;

		/**
		 * @brief A constructor for pleasent dimensions of the window
		 *
		 * @since Karma 1.0.0
		 */
		WindowProps(const std::string& title = "Karma Engine",
			unsigned int width = 1280,
			unsigned int height = 720) : Title(title),
			Width(width), Height(height)
		{
		}
	};

	// Interface representing desktop based window
	/**
	 * @brief The abstract base class of Karma's window (for platform specific purposes)
	 */
	class KARMA_API Window
	{
	public:
		/**
		 * @brief A data structure for use in Window::SetEventCallback
		 *
		 * For instance KR_BIND_EVENT_FN(Application::OnEvent) is a EventCallbackFn.
		 *
		 * @since Karma 1.0.0
		 */
		using EventCallbackFn = std::function<void(Event&)>;
		
		/**
		 * @brief A virtual destructor for the Window
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~Window()
		{
		}
		
		/**
		 * @brief Pure virtual function for calls in each loop
		 *
		 * @see Application::Run()
		 * @since Karma 1.0.0
		 */
		virtual void OnUpdate() = 0;
		
		/**
		 * @brief Pure virtual function called when Window resize happens
		 *
		 * @see Application::OnWindowResize
		 * @since Karma 1.0.0
		 */
		virtual bool OnResize(WindowResizeEvent& event) = 0;
		
		/**
		 * @brief A pure virtual getter for Width of the Window
		 *
		 * @since Karma 1.0.0
		 */
		virtual unsigned int GetWidth() const = 0;
		
		/**
		 * @brief Pure virtual getter for the Height of the Window
		 *
		 * @since Karma 1.0.0
		 */
		virtual unsigned int GetHeight() const = 0;
		
		/**
		 * @brief Setting a function, Application::OnEvent, to be called when a Karma Event happens, detected by GLFW Window like so
		 * @code{.cpp}
		 * SetGLFWCallbacks(GLFWwindow* glfwWindow)
		 * {
		 * 		//
		 * 		//	@param GLFWwindow the window whose sizecallback is desired
		 * 		//	@param GLFWwindowsizefun the function pointer that gets called whenever
		 * 		//			window size changes. The Cherno uses something what is
		 * 		//			known as lambda. I don't quite understand the relation
		 * 		//			between function pointer and lambda.
		 * 		//
		 * 		glfwSetWindowSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height)
		 * 		{
		 * 			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		 * 			data.Width = width;
		 * 			data.Height = height;
		 * 			WindowResizeEvent event(width, height);
		 * 			data.EventCallback(event);// <-- This is translated to Application::OnEvent(event)
		 * 		});
		 * }
		 *
		 * @endcode
		 *
		 * Setting the listener basically.
		 *
		 * @param callback						The function to be called
		 *
		 * @see Application::Application()
		 * @since Karma 1.0.0
		 */
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		
		/**
		 * @brief Pure virtual function for VSync
		 *
		 * @since Karma 1.0.0
		 */
		virtual void SetVSync(bool enabled) = 0;
		
		/**
		 * @brief Pure virtual function for VSync status
		 *
		 * @since Karma 1.0.0
		 */
		virtual bool IsVSync() const = 0;
		
		/**
		 * @brief Pure virtual getter for the native (GLFW) Window handle
		 *
		 * @since Karma 1.0.0
		 */
		virtual void* GetNativeWindow() const = 0;
		
		/**
		 * @brief Platform based, implemented function for Window creation.
		 *
		 * @since Karma 1.0.0
		 */
		static Window* Create(const WindowProps& props = WindowProps());
	};
}
