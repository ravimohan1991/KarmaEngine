#include "Buffer.h"
#include "Karma/Core.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Karma
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				return nullptr;
		case RendererAPI::API::OpenGL:
				return new OpenGLVertexBuffer(vertices, size);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI specified");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				return nullptr;
		case RendererAPI::API::OpenGL:
				return new OpenGLIndexBuffer(indices, size);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI specified");
		return nullptr;
	}
}