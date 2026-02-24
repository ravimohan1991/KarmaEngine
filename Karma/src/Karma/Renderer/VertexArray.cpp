#include "VertexArray.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Karma
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return new OpenGLVertexArray();
			case RendererAPI::API::Vulkan:
				//return new VulkanVertexArray();
				return nullptr;
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
