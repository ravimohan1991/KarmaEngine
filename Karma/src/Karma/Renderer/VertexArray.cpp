#include "VertexArray.h"
#include "Renderer.h"
#include "Karma/Core.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Karma
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
			return nullptr;
		case RendererAPI::OpenGL:
			return new OpenGLVertexArray();
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}