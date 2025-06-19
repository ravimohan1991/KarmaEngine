/**
 * @file LinuxWindow.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains LinuxWindow class and relevant data structures.
 * @version 1.0
 * @date December 13, 2020
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
	 * @brief Linux specific Window class
	 */
	class LinuxWindow : public Window
	{
	public:
		/**
		 * @brief A constructor to initialize Linux Window with appropriate properties
		 *
		 * @param props						The properties with which the window is to be initialized
		 * @since Karma 1.0.0
		 */
		LinuxWindow(const WindowProps& props);

		/**
		 * @brief A destructor to clean up the mess and Linux window relevant resources.
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~LinuxWindow();

		/**
		 * @brief Called each loop for input polling and OpenGL swapbuffers
		 *
		 * @see Application::Run()
		 * @since Karma 1.0.0
		 */
		void OnUpdate() override;

		/**
		 * @brief Linux implementation of WindowResize event.
		 *
		 * @since Karma 1.0.0
		 */
		virtual bool OnResize(WindowResizeEvent& event) override;

		/**
		 * @brief Getter for the Linux Window width.
		 *
		 * @since Karma 1.0.0
		 */
		inline unsigned int GetWidth() const override { return m_Data.Width; }

		/**
		 * @brief Getter for Linux Window height.
		 *
		 * @since Karma 1.0.0
		 */
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		/**
		 * @brief Sets a listner for Linux Window
		 *
		 * @param callback						The function to be called
		 * @since Karma 1.0.0
		 */
		inline void SetEventCallback(const EventCallbackFn& callback) override
		{
			m_Data.EventCallback = callback;
		}

		/**
		 * @brief Getter for the Linux Window handle.
		 *
		 * @since Karma 1.0.0
		 */
		inline virtual void* GetNativeWindow() const override { return m_Window; }

		/**
		 * @brief Based on API chosen, the Linux based VSync toggle.
		 *
		 * @since Karma 1.0.0
		 */
		void SetVSync(bool enabled) override;

		/**
		 * @brief Query for Linux VSync status.
		 *
		 * @since Karma 1.0.0
		 */
		bool IsVSync() const override;

	private:

		/**
		 * @brief Initializes Windows on Linux platform
		 *
		 * The following steps are taken:
		 * 1. The (WindowData) m_Data is filled with appropriate value(s) from props
		 * 2. GLFW is initialized if not already
		 * 3. Based on rendering API, create a context
		 * 4. Set Event callbacks (relevant to LinuxWindow)
		 * 5. Set the LinuxWindow icon.
		 *
		 * @since Karma 1.0.0
		 */
		void Init(const WindowProps& props);

		/**
		 * @brief Sets the GLFW based callbacks
		 * The details of the callback are like so:
		 * 1. The lambdas containing the function call (EventCallbackFn) for the following events
		 *		WindowResizeEvent, WindowCloseEvent, KeyPressedEvent, KeyReleasedEvent, KeyTypedEvent, MouseButtonPressedEvent,
		 *		MouseButtonReleasedEvent, MouseScrolledEvent, MouseMovedEvent.
		 *
		 * @param glfwWindow				The GLFW LinuxWindow handle
		 *
		 * @note GLFW callbacks for Input (different from KeyBoard and Mouse) are not set here.
		 * @see LinuxInput::SetEventCallback
		 * 
		 * @since Karma 1.0.0
		 */
		void SetGLFWCallbacks(GLFWwindow* glfwWindow);

		/**
		 * @brief Destroys the LinuxWindow and does necessary cleanups
		 *
		 * @see ~LinuxWindow()
		 * @since Karma 1.0.0
		 */
		void ShutDown();

		GLFWwindow* m_Window;
		GraphicsContext* m_Context;

		/**
		 * @brief A structure for containing data for LinuxWindow
		 *
		 * @note The data could be expanded in future.
		 */
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
