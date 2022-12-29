#pragma once

#include "krpch.h"

#include "RenderCommand.h"
#include "Karma/Renderer/Camera/Camera.h"
#include "glm/glm.hpp"
#include "Renderer/Scene.h"

namespace Karma
{
	class KARMA_API Renderer
	{
	public:
		static void SetScene(std::shared_ptr<Scene> sceneToSet) { m_Scene = sceneToSet; }

		static void BeginScene(std::shared_ptr<Scene> scene);
		static void EndScene();

		static void Submit(std::shared_ptr<Scene> scene);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void DeleteData();

		// Getter
		static std::shared_ptr<Scene> GetScene() { return m_Scene; }

	private:
		// Needs to be in Scene class
		struct SceneData
		{
			glm::mat4 ProjectionMatrix;
		};

		static SceneData* m_SceneData;
		static std::shared_ptr<Scene> m_Scene;
	};
}