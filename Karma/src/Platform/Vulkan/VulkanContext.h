#pragma once

#define GLFW_INCLUDE_VULKAN
#include "Karma/Core.h"
#include "Karma/Renderer/GraphicsContext.h"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

struct GLFWwindow;

namespace Karma
{
	class KARMA_API VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext() override;

		virtual void Init() override;
		virtual void SwapBuffers() override;

		void CreateInstance();

	private:
		GLFWwindow* m_windowHandle;
		VkInstance instance;
	};
}