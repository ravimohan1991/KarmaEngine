/**
 * @file RenderCommand.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class RenderCommand.
 * @version 1.0
 * @date December 25, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "RendererAPI.h"

namespace Karma
{
	/**
	 * @brief A class with static routines relevant for rendering a scene using RendererAPI
	 */
	class KARMA_API RenderCommand
	{
	public:
		/**
		 * @brief Initializing RenderCommand by creating the instance of appropriate RendererAPI
		 * Called in the main function at the moment
		 *
		 * @see RendererAPI::API
		 * @since Karma 1.0.0
		 */
		static void Init();

		/**
		 * @brief Deinitialize RenderCommand by freeing up rendering resources
		 *
		 * @since Karma 1.0.0
		 */
		static void DeInit();
		
		/**
		 * @brief Sets the background color of the screen
		 *
		 * @since Karma 1.0.0
		 */
		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		/**
		 * @brief Clears the rendering screen
		 *
		 * @todo Seems like VulkanRendererAPI is not using this function call for clearing.
		 *
		 *
		 * @since Karma 1.0.0
		 */
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		/**
		 * @brief Setting up resources for rendering of a scene
		 *
		 * Usually the sequence for rendering frame goes like
		 *
		 *	@code{.cpp}
		 *		Karma::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });
		 *		Karma::RenderCommand::Clear();
		 *		Karma::Renderer::BeginScene(m_Scene);
		 *		KR_INFO("DeltaTime = {0} ms", deltaTime * 1000.0f);
		 *		m_SquareVA->UpdateProcessAndSetReadyForSubmission();
		 *		m_SquareVA->Bind();
		 *		Karma::Renderer::Submit(m_Scene);
		 *		Karma::Renderer::EndScene();
		 *	@endcode
		 *
		 * @since Karma 1.0.0
		 */
		inline static void BeginScene()
		{
			s_RendererAPI->BeginScene();
		}

		/**
		 * @brief Actually issue a draw call of primitives with indexed vertices
		 *
		 * For OpenGL the command is glDrawElements
		 * For Vulkan the command is vkCmdDrawIndexed
		 *
		 * @param vertexArray				The mesh + material to be rendered
		 *
		 * @since Karma 1.0.0
		 */
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

		/**
		 * @brief The clearing of resources, if any, at the end of frame
		 *
		 * @since Karma 1.0.0
		 */
		static void EndScene()
		{
			s_RendererAPI->EndScene();
		}
		
		/**
		 * @brief Getter for the set clear color
		 *
		 * @since Karma 1.0.0
		 */
		inline static glm::vec4& GetClearColor()
		{
			return s_RendererAPI->GetClearColor();
		}

		/**
		 * @brief Getter for the renderer API
		 *
		 * @since Karma 1.0.0
		 */
		inline static RendererAPI* GetRendererAPI() { return s_RendererAPI; }

	private:
		static RendererAPI* s_RendererAPI;
	};
}
