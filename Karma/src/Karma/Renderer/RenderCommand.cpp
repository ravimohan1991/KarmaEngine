#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Karma
{
	RendererAPI* RenderCommand::s_RendererAPI = nullptr;

	void RenderCommand::Init()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
			break;
		case RendererAPI::API::OpenGL:
			s_RendererAPI = new OpenGLRendererAPI();
			break;
		case RendererAPI::API::Vulkan:
			s_RendererAPI = new VulkanRendererAPI();
			break;
		default:
			KR_CORE_ASSERT(false, "Unknown RendererAPI specified");
			break;
		}

		KR_CORE_INFO("Initialized RenderCommand");
	}

	void RenderCommand::DeInit()
	{
		delete s_RendererAPI;
		s_RendererAPI = 0;

		KR_CORE_INFO("Deinitialized RenderCommand");
	}
}