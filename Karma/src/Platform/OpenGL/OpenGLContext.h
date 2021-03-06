#pragma once

#include "Karma/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Karma
{
	class OpenGLContext : public GraphicsContext
	{
	public: 
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_windowHandle;
	};
}