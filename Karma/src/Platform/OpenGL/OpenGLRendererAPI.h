/**
 * @file OpenGLRendererAPI.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains OpenGLRendererAPI class
 * @version 1.0
 * @date December 25, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Renderer/RendererAPI.h"

namespace Karma
{
	/**
	 * @brief OpenGL renderer
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API OpenGLRendererAPI : public RendererAPI
	{
	public:
		/**
		 * @brief Set the color to be used for clear (rendering) screen
		 *
		 * @param color				The color to be used for clearing
		 * @since Karma 1.0.0
		 */
		virtual void SetClearColor(const glm::vec4& color) override;

		/**
		 * @brief Clear the rendering screen
		 *
		 * @since Karma 1.0.0
		 */
		virtual void Clear() override;

		/**
		 * @brief In OpenGL left empty
		 *
		 * @since Karma 1.0.0
		 */
		virtual void BeginScene() override {}

		/**
		 * @brief Do the triangle drawing using glDrawElements
		 *
		 * @param vertexArray						The format and relevant information of the rendrable vertex data
		 *
		 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
		 * @since Karma 1.0.0
		 */
		virtual void DrawIndexed(const std::shared_ptr<VertexArray> vertexArray) override;

		/**
		 * @brief In openGL left empty
		 *
		 * @since Karma 1.0.0
		 */
		virtual void EndScene() override {}
	};
}
