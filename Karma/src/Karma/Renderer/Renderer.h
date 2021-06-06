#pragma once

#include "Karma/Core.h"
#include "RenderCommand.h"
#include "Karma/Renderer/Camera/Camera.h"
#include "Karma/Renderer/Shader.h"
#include "glm/glm.hpp"

namespace Karma
{
	class KARMA_API Renderer
	{
	public:
		static void BeginScene(Camera& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader, glm::mat4& transform = glm::mat4(1.0f));

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