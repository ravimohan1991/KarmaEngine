/**
 * @file Renderer.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class Renderer.
 * @version 1.0
 * @date December 21, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "RenderCommand.h"
#include "Karma/Renderer/Camera/Camera.h"
#include "glm/glm.hpp"
#include "Renderer/Scene.h"

namespace Karma
{
	/**
	 * @brief An overlay, if I may consider, for RenderCommand, used for rendering a scene using a renderer (vulkan / opengl)
	 */
	class KARMA_API Renderer
	{
	public:
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
		static void BeginScene(std::shared_ptr<Scene> scene);

		/**
		 * @brief The clearing of resources, if any, at the end of frame
		 *
		 * @since Karma 1.0.0
		 */
		static void EndScene();

		/**
		 * @brief Submitting a scene for rendering
		 *
		 * @since Karma 1.0.0
		 */
		static void Submit(std::shared_ptr<Scene> scene);

		/**
		 * @brief Getter for the rendering API
		 *
		 * @since Karma 1.0.0
		 */
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		/**
		 * @brief Clearing scenedata
		 *
		 * @since Karma 1.0.0
		 */
		static void DeleteData();

	private:
		// Needs to be in Scene class
		struct SceneData
		{
			glm::mat4 ProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}
