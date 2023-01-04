#pragma once

#include "krpch.h"

#include "Camera.h"
#include "VertexArray.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace Karma
{
	class KARMA_API Scene
	{
	public:
		Scene();
		~Scene();

		void AddVertexArray(std::shared_ptr<VertexArray> vertexArray);
		void AddCamera(std::shared_ptr<Camera> camera);

		void SetClearColor(const glm::vec4& clearColor) { m_ClearColor = clearColor; }
		void SetRenderWindow(ImGuiWindow* window) { m_WindowToRenderWithin = window; };

		// Getters
		std::shared_ptr<VertexArray> GetRenderableVertexArray() const;
		std::shared_ptr<Camera> GetSceneCamera() const;

		const glm::vec4& GetClearColor() const { return m_ClearColor; }

		const std::vector<std::shared_ptr<VertexArray>>& GetAllVertexArrays() const { return m_VertexArrays; }
		const std::vector<std::shared_ptr<Camera>>& GetAllCameras() const { return m_Cameras; }

		inline ImGuiWindow* GetRenderingWindow() const { return m_WindowToRenderWithin; }

	private:
		std::vector<std::shared_ptr<VertexArray>> m_VertexArrays;
		std::vector<std::shared_ptr<Camera>> m_Cameras;

		glm::vec4 m_ClearColor;
		
		// Caution: raw pointer, courtsey authors of Dear ImGui
		ImGuiWindow* m_WindowToRenderWithin;
	};
}