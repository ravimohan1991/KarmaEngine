#include "VertexArray.h"
#include "KarmaRHI/DynamicRHI.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Karma
{
	VertexArray* VertexArray::Create()
	{
        switch (GRHIInterfaceType)
		{
            case ERHIInterfaceType::Null:
                KR_CORE_ASSERT(false, "Without graphics API currently not supported");
				return nullptr;
            case ERHIInterfaceType::OpenGL:
				return new OpenGLVertexArray();
            case ERHIInterfaceType::Vulkan:
				//return new VulkanVertexArray();
				return nullptr;
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
