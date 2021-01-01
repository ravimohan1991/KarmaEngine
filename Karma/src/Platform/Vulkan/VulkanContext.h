#pragma once

#include "Karma/Core.h"
#include "Karma/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Karma
{
	class KARMA_API VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_windowHandle;
	};
}