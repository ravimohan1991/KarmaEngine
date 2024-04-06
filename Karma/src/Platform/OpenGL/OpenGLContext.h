/**
 * @file OpenGLContext.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains OpenGLContext class
 * @version 1.0
 * @date December 18, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Karma
{
	/**
	 * @brief OpenGL API based implementation of GraphicsContext
	 *
	 * @since Karma 1.0.0
	 */
	class OpenGLContext : public GraphicsContext
	{
	public: 
		/**
		 * @brief A constructor
		 *
		 * Sets the glfw window handle and null asserts
		 *
		 * @param windowHandle						The window handle to be set to, in the context
		 *
		 * @since Karma 1.0.0
		 */
		OpenGLContext(GLFWwindow* windowHandle);

		/**
		 * @brief Initializes the context
		 *
		 * Loads Glad
		 *
		 * @since Karma 1.0.0
		 */
		virtual void Init() override;

		/**
		 * @brief This function swaps the front and back buffers of the context window. 
		 *
		 * @note If the swap interval is greater than zero, the GPU driver waits the specified number of screen updates before swapping the buffers.
		 * @since Karma 1.0.0
		 */
		virtual void SwapBuffers() override;
		
		/**
		 * @brief Called by Window::Resize, when the resize of a window happens
		 *
		 * @param event						The details of the resizing event
		 * @since Karma 1.0.0
		 */
		virtual bool OnWindowResize(WindowResizeEvent& event) override;

	private:
		GLFWwindow* m_windowHandle;
	};
}
