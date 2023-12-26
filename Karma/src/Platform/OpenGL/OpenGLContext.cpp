#include "OpenGLContext.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Karma/Core.h"

namespace Karma
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_windowHandle(windowHandle)
	{
		KR_CORE_ASSERT(windowHandle, "windowHandle is null");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);// Loads OpenGL extensions. Windows made OpenGL free!

		KR_CORE_ASSERT(status, "Failed to initialize Glad");
		KR_CORE_INFO("Glad initialized with OpenGL version {0}", (const char *) glGetString(GL_VERSION));
	}

	// Based on the advice from
	// https://github.com/glfw/glfw/issues/1016#issuecomment-320526521
	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_windowHandle);
		glfwSwapInterval(1);
		
		glFinish();
	}

	bool OpenGLContext::OnWindowResize(WindowResizeEvent& event)
	{
		glViewport(0, 0, event.GetWidth(), event.GetHeight());
		return true;
	}
}
