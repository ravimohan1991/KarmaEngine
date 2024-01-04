/**
 * @file Scene.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class Scene.
 * @version 1.0
 * @date December 29, 2022
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Camera.h"
#include "VertexArray.h"

namespace Karma
{
	/**
	 * @brief Class for organizing and containing Scene relevant data
	 */
	class KARMA_API Scene
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		Scene();

		/**
		 * @brief Destructor
		 *
		 * @since Karma 1.0.0
		 */
		~Scene();

		/**
		 * @brief Add a VertexArray (Mesh + Material) to the list of renderable VertexArrays
		 *
		 * @param vertexArray				Adding Mesh + Material to be rendered
		 * @since Karma 1.0.0
		 */
		void AddVertexArray(std::shared_ptr<VertexArray> vertexArray);

		/**
		 * @brief Add Camera
		 *
		 * @param camera				The Camera to be added
		 * @since Karma 1.0.0
		 */
		void AddCamera(std::shared_ptr<Camera> camera);

		/**
		 * @brief Set the m_ClearColor variable
		 *
		 * @param clearColor				This color to be set
		 * @since Karma 1.0.0
		 */
		void SetClearColor(const glm::vec4& clearColor) { m_ClearColor = clearColor; }

		/**
		 * @brief Set the Window in which the Scene is to be rendered
		 *
		 * Could be a KGGuiWindow if rendering in the Editor.
		 * For standalone game, maybe Window can be used.
		 *
		 * @param window					The window for rendering
		 *
		 * @see KarmaGuiMesa::Draw3DModelExhibitorMesa
		 * @since Karma 1.0.0
		 */
		void SetRenderWindow(void* window) { m_WindowToRenderWithin = window; };

		/**
		 * @brief Sets the status of the m_WindowResize true if resize is done so that necessary reset and reallocation of resources
		 * can be done.
		 *
		 * @param bStatus						True if resize is done and rendering resources are to be reallocated
		 * @since Karma 1.0.0
		 */
		void SetWindowToRenderWithinResize(bool bStatus) { m_WindowResize = bStatus; }

		// Getters
		/**
		 * @brief Getter for the VertexArray
		 *
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<VertexArray> GetRenderableVertexArray() const;

		/**
		 * @brief Getter for the Camera being used for the scene
		 *
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<Camera> GetSceneCamera() const;

		/**
		 * @brief Getter for the m_ClearColor
		 *
		 * @since Karma 1.0.0
		 */
		inline const glm::vec4& GetClearColor() const { return m_ClearColor; }

		/**
		 * @brief Get the list of VertexArrays
		 *
		 * @since Karma 1.0.0
		 */
		inline const std::vector<std::shared_ptr<VertexArray>>& GetAllVertexArrays() const { return m_VertexArrays; }

		/**
		 * @brief Get the list of all Cameras
		 *
		 * @since Karma 1.0.0
		 */
		inline const std::vector<std::shared_ptr<Camera>>& GetAllCameras() const { return m_Cameras; }

		/**
		 * @brief Getter for the window in which the scene is rendered
		 *
		 * @since Karma 1.0.0
		 */
		inline void* GetRenderingWindow() const { return m_WindowToRenderWithin; }

		/**
		 * @brief Getter for the status of wether the window resize is to be handled
		 *
		 * @since Karma 1.0.0
		 */
		inline bool GetWindowToRenderWithinResizeStatus() const { return m_WindowResize; }

	private:
		std::vector<std::shared_ptr<VertexArray>> m_VertexArrays;
		std::vector<std::shared_ptr<Camera>> m_Cameras;

		glm::vec4 m_ClearColor;
		
		// Caution: raw pointer, courtsey authors of Dear ImGui
		void* m_WindowToRenderWithin;
		bool m_WindowResize;
	};
}
