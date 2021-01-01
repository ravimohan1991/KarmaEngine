#include "VulkanContext.h"
#include "GLFW/glfw3.h"

namespace Karma
{
	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_windowHandle(windowHandle)
	{
		KR_CORE_ASSERT(windowHandle, "windowHandle is null");
	}

	void VulkanContext::Init()
	{
	}

	void VulkanContext::SwapBuffers()
	{
	}
}