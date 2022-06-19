#include "LinuxInput.h"
#include "GLFW/glfw3.h"
#include "Karma/Application.h"

namespace Karma
{
    
    LinuxInput::LinuxInput() : m_Data(), Input(m_Data)
    {

    }

    void LinuxInput::SetEventCallback(const EventCallbackFn& callback, std::shared_ptr<Window> window)
    {

    }

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

    bool LinuxInput::IsControllerButtonPressedImpl(int button, int cID)
    {
        return false;
    }

    bool LinuxInput::IsMouseButtonReleasedImpl(int button)
    {
        return false;
    }

    float LinuxInput::ControllerAxisPivotValImpl(int axis, int cID)
    {
        return 0.0f;
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
