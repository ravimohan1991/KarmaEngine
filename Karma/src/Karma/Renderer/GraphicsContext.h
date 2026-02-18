/**
 * @file GraphicsContext.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains base class GraphicsContext
 * @version 1.0
 * @date December 18, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Events/ApplicationEvent.h"

namespace Karma
{
	/**
	 * @brief An abstract class for creating a context for Renderer and provide graphics API
	 */
	class KARMA_API GraphicsContext
	{
	public:
		/**
		 * @brief For initializing Renderer and instantiating relevant objects
		 *
		 * @since Karma 1.0.0
		 */
		virtual void Init() = 0;

		/**
		 * @brief This function swaps the front and back buffers of the specified window. If the swap interval is greater than zero, the GPU driver waits the specified number of screen updates before swapping the buffers.
		 *
		 * @todo This seems to be an OpenGL specific call. Need to design api accordingly
		 * @since Karma 1.0.0
		 */
		virtual void SwapBuffers() = 0;

		/**
		 * @brief Calls glViewport function which specifies the affine transformation of x and y from normalized device coordinates to window coordinates
		 *
		 * @todo Seems OpenGL specific. Either think about usage in Vulkan or re design
		 * @since Karma 1.0.0
		 */
		virtual bool OnWindowResize(WindowResizeEvent& event) = 0;

		/**
		 * @brief A destructor
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~GraphicsContext()
		{
		}
	};
}
