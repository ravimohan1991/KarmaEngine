/**
 * @file WindowsWindow.h
 * @author Ravi Mohan (the_cowboy)
 * @brief Windows platform specific implementation of the Window class
 * @version 1.0
 * @date November 29, 2020
 * 
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Window.h"

struct GLFWwindow;

namespace Karma
{
	class GraphicsContext;
	
	/**
	 * @brief Windows specific implementation of the Window class
	 * 
	 * @since Karma 1.0.0
	 */
	class WindowsWindow : public Window
	{
	public:
		/**
		 * @brief A constructor
		 * 
		 * @param props						The properties of the window to be created
		 * 
		 * @see WindowsWindow::Init
		 * @since Karma 1.0.0
		 */
		WindowsWindow(const WindowProps& props);
		
		/**
		 * @brief A destructor
		 * 
		 * @see WindowsWindow::ShutDown()
		 * @since Karma 1.0.0
		 */
		virtual ~WindowsWindow();
		
		/**
		 * @brief Function called in each loop iteration
		 * 
		 * @see Application::Run()
		 * @since Karma 1.0.0
		 */
		virtual void OnUpdate() override;
		
		/**
		 * @brief Windows specific implementation of WindowResize event.
		 * 
		 * @param event						The WindowResizeEvent containing relevant data
		 * @since Karma 1.0.0
		 */
		virtual bool OnResize(WindowResizeEvent& event) override;

		/**
		 * @brief Getter for the Window width
		 * 
		 * @return unsigned int				The width of the window
		 * @since Karma 1.0.0
		 */
		inline unsigned int GetWidth() const override { return m_Data.Width; }
		
		/**
		 * @brief Getter for the Window height
		 * 
		 * @return unsigned int				The height of the window
		 * @since Karma 1.0.0
		 */
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		/**
		 * @brief Sets a listener for Window events
		 * 
		 * @param callback					The function to be called on event
		 * 
		 * @see Application::Application()
		 * @since Karma 1.0.0
		 */
		inline void SetEventCallback(const EventCallbackFn& callback) override
		{
			m_Data.EventCallback = callback;
		}

		/**
		 * @brief Getter for the native Window handle
		 * 
		 * @return void*					The native window handle
		 * @since Karma 1.0.0
		 */
		inline virtual void* GetNativeWindow() const override { return m_Window; }

		/**
		 * @brief Sets VSync on or off
		 * 
		 * @param enabled					True to enable VSync, false to disable
		 * @since Karma 1.0.0
		 */
		void SetVSync(bool enabled) override;

		/**
		 * @brief Getter for VSync status
		 * 
		 * @return bool						True if VSync is enabled, false otherwise
		 * @since Karma 1.0.0
		 */
		bool IsVSync() const override;

		/**
		 * @brief Getter for the GLFW Window handle
		 * 
		 * @return GLFWwindow*				The GLFW window handle
		 * @since Karma 1.0.0
		 */
		GLFWwindow* GetHandle() const { return m_Window; }

	private:
		void Init(const WindowProps& props);

		void SetGLFWCallbacks(GLFWwindow* glfwWindow);
		void ShutDown();

		GLFWwindow* m_Window;
		GraphicsContext* m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}