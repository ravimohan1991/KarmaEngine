#pragma once

#include "Karma/Core.h"

namespace Karma
{
	enum class RendererAPI
	{
		None = 0,
		OpenGL = 1
	};

	class KARMA_API Renderer
	{
	public:
		inline static RendererAPI GetAPI() { return s_RendererAPI; }

	private:
		static RendererAPI s_RendererAPI;
	};
}