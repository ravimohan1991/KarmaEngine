#pragma once

#include "Karma/Core.h"
#include "RenderCommand.h"
#include "Karma/Renderer/OrthographicCamera.h"
#include "Karma/Renderer/Shader.h"
#include "glm/glm.hpp"

namespace Karma
{
	/*enum class RendererAPI
	{
		None = 0,
		OpenGL = 1
	};*/

	class KARMA_API Renderer
	{
	public:
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI();}

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}