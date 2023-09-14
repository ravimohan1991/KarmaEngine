#pragma once

#include "krpch.h"

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
		inline static void BeginScene()
		{
			s_RendererAPI->BeginScene();
		}
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}
		static void EndScene()
		{
			s_RendererAPI->EndScene();
		}
		
		inline static glm::vec4& GetClearColor()
		{
			return s_RendererAPI->GetClearColor();
		}

		inline static RendererAPI* GetRendererAPI() { return s_RendererAPI; }

	private:
		static RendererAPI* s_RendererAPI;
	};
}
