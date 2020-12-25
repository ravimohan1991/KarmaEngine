#pragma once

#include "Karma/Core.h"
#include "RenderCommand.h"

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
		static void BeginScene();
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI();}
	};
}