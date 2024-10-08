#include "WindowsWindow.h"
#include "Karma/Log.h"
#include "Karma/Events/ApplicationEvent.h"
#include "Karma/Events/KeyEvent.h"
#include "Karma/Events/MouseEvent.h"
#include "GLFW/glfw3.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Karma/Renderer/Renderer.h"

namespace Karma
{
	static bool s_GLFWInitialized = false;
	static void GLFWErrorCallback(int error, const char* message)
	{
		KR_CORE_ERROR("GLFW error ({0}) : {1}", error, message);
	}

#ifdef KR_WINDOWS_PLATFORM
	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}
#endif

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		ShutDown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;

		// Need to find a algorithm for gauging the default window resolution and work the dimensions accrodingly
		m_Data.Width = 2 * props.Width;
		m_Data.Height = 2 * props.Height;

		KR_CORE_INFO("Creating Windows window {0} ({1}, {2})", props.Title, m_Data.Width, m_Data.Height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			KR_CORE_ASSERT(success, "GLFW not initialized");

			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		RendererAPI::API currentAPI = RendererAPI::GetAPI();

		if (currentAPI == RendererAPI::API::Vulkan)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			uint32_t extensionCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
			KR_CORE_INFO("{0} Vulkan extensions supported", extensionCount);
		}

		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

		switch (currentAPI)
		{
		case RendererAPI::API::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
			break;
		case RendererAPI::API::OpenGL:
			m_Context = new OpenGLContext(m_Window);
			break;
		case RendererAPI::API::Vulkan:
			m_Context = new VulkanContext(m_Window);
			break;
		}

		m_Context->Init();
		SetVSync(true);

		// Used for event callbacks
		glfwSetWindowUserPointer(m_Window, &m_Data);

		// Set glfw callbacks
		SetGLFWCallbacks(m_Window);

		// Set the ICOOOOON
		GLFWimage karmaEQ;
		karmaEQ.pixels = stbi_load("../Resources/Textures/KarmaEQ.png", &karmaEQ.width, &karmaEQ.height, 0, 4); //rgba channels
		glfwSetWindowIcon(m_Window, 1, &karmaEQ);
		stbi_image_free(karmaEQ.pixels);
	}

	bool WindowsWindow::OnResize(WindowResizeEvent& event)
	{
		return m_Context->OnWindowResize(event);
	}

	void WindowsWindow::SetGLFWCallbacks(GLFWwindow* glfwWindow)
	{
		/*
		*	@param GLFWwindow the window whose sizecallback is desired
		*	@param GLFWwindowsizefun the function pointer that gets called whenever
		*			window size changes. The Cherno uses something what is
		*			known as lambda. I don't quite understand the relation
		*			between function pointer and lambda.
		*/
		glfwSetWindowSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(glfwWindow, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;

			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, 1);
				data.EventCallback(event);
				break;
			}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void WindowsWindow::ShutDown()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		if (m_Context)
		{
			delete m_Context;
			m_Context = 0;
		}
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		RendererAPI::API currentAPI = RendererAPI::GetAPI();

		switch (currentAPI)
		{
		case RendererAPI::API::OpenGL:
		{
			if (enabled)
			{
				glfwSwapInterval(1);
			}
			else
			{
				glfwSwapInterval(0);
			}
			break;
		}
		case RendererAPI::API::Vulkan:
		{
			VulkanContext* vContext = static_cast<VulkanContext*>(m_Context);
			vContext->SetVSync(enabled);
			break;
		}
		case RendererAPI::API::None:
		{
			KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
			break;
		}
		}
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
}