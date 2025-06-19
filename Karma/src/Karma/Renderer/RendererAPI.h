/**
 * @file RendererAPI.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class RendererAPI.
 * @version 1.0
 * @date December 25, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "glm/glm.hpp"
#include "VertexArray.h"

namespace Karma
{
	/**
	 * @brief An abstract class for a renderer
	 */
	class KARMA_API RendererAPI
	{
	public:
		/**
		 * @brief The rendering API used by the Engine
		 */
		enum class API
		{
			/**
			 * @brief No rendering
			 * @todo None rendering could be supported in future
			 */
			None = 0,
			/**
			 * @brief OpenGL (https://www.opengl.org)
			 */
			OpenGL = 1,
			/**
			 * @brief Vulkan (https://www.vulkan.org)
			 */
			Vulkan = 2
		};

	public:
		/**
		 * @brief A destructor
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~RendererAPI() {}

		/**
		 * @brief Set the color to be used for clear (rendering) screen
		 *
		 * @param color				The color to be used for clearing
		 * @since Karma 1.0.0
		 */
		virtual void SetClearColor(const glm::vec4& color) = 0;

		/**
		 * @brief Clear the rendering screen
		 *
		 * @since Karma 1.0.0
		 */
		virtual void Clear() = 0;

		/**
		 * @brief Setting up resources for rendering of a scene
		 *
		 * @see RenderCommand::BeginScene
		 * @since Karma 1.0.0
		 */
		virtual void BeginScene() = 0;

		/**
		 * @brief Routine for drawing primitives
		 *
		 * @since Karma 1.0.0
		 */
		virtual void DrawIndexed(std::shared_ptr<VertexArray> vertexArray) = 0;

		/**
		 * @brief Instructions for end of the scene
		 *
		 * @since Karma 1.0.0
		 */
		virtual void EndScene() = 0;
		
		/**
		 * @brief Getter for m_ClearColor variable
		 *
		 * @since Karma 1.0.0
		 */
		inline glm::vec4& GetClearColor() const { return m_ClearColor; }

		/**
		 * @brief Getter for rendering api being used
		 *
		 * @since Karma 1.0.0
		 */
		inline static API GetAPI() { return s_API; }

	private:
		static API s_API;
		
	protected:
		// Need to see the utility
		static glm::vec4 m_ClearColor;
	};
}
