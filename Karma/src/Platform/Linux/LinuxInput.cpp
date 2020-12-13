#include "LinuxInput.h"
#include "GLFW/glfw3.h"
#include "Karma/Application.h"

namespace Karma
{
#ifdef KR_LINUX_PLATFORM
	Input* Input::s_Instance = new LinuxInput();
#endif
    
    bool LinuxInput::IsKeyPressedImpl(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool LinuxInput::IsMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);

		return state == GLFW_PRESS;
	}

	std::pair<float, float> LinuxInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float LinuxInput::GetMouseXImpl()
	{
		/*
		* for c++14 and below (I am supporting c++11 in linux
		* so that is the bottleneck for Karma. https://youtu.be/yuhNj8yGDJQ?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&t=1302)
		* auto v = GetMousePositionImpl();
		* return std::get<0>(v);
		*/

		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return (float)xpos;
	}

	float LinuxInput::GetMouseYImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return (float)ypos;
	}
}