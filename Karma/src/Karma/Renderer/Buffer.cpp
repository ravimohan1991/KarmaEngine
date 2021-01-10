#include "Buffer.h"
#include "Karma/Core.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

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
			case RendererAPI::API::Vulkan:
				return new VulkanVertexBuffer(vertices, size);
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
			case RendererAPI::API::Vulkan:
				return new VulkanIndexBuffer(indices, size);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI specified");
		return nullptr;
	}
}