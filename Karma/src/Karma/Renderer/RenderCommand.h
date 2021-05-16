#pragma once

#include "Karma/Core.h"
#include "RendererAPI.h"

namespace Karma
{
	class KARMA_API RenderCommand
	{
	public:
		static void Init();
		static void DeInit();
		
		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}


		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

		inline static RendererAPI* GetRendererAPI() { return s_RendererAPI; }

	private:
		static RendererAPI* s_RendererAPI;
	};
}