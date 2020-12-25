#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Karma
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}