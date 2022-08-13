#pragma once

#include "krpch.h"

#include "RenderCommand.h"
#include "Karma/Renderer/Camera/Camera.h"
#include "glm/glm.hpp"

namespace Karma
{
	class KARMA_API Renderer
	{
	public:
		static void BeginScene(Camera& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI();}

		static void DeleteData();

	private:
		struct SceneData
		{
			glm::mat4 ProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}