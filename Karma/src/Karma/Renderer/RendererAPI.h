#pragma once

#include "krpch.h"

#include "glm/glm.hpp"
#include "VertexArray.h"

namespace Karma
{
	class KARMA_API RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1,
			Vulkan = 2
		};

	public:
		virtual ~RendererAPI() {}
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void BeginScene() = 0;
		virtual void DrawIndexed(std::shared_ptr<VertexArray> vertexArray) = 0;
		virtual void EndScene() = 0;

		inline static API GetAPI() { return s_API; }

	private:
		static API s_API;
	};
}