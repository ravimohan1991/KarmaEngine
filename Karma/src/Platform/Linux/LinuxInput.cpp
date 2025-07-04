#include "LinuxInput.h"
#include "GLFW/glfw3.h"

#include "Karma/Application.h"

namespace Karma
{

	LinuxInput::LinuxInput() : m_Data(), Input(m_Data)
	{
	}

	void LinuxInput::SetEventCallback(const EventCallbackFn& callback, Window* window)
	{
		m_Data.EventCallback = callback;

		if (Input::GetAPI() == InputRegisteringAPI::GlfwInput)
		{
			glfwSetJoystickCallback([](int cID, int event)
			{
				if (event == GLFW_DISCONNECTED)
				{
					ControllerDeviceDisconnectedEvent eve(cID, event);

					EventDispatcher dispatcher(eve);
					dispatcher.Dispatch<ControllerDeviceDisconnectedEvent>([](ControllerDeviceDisconnectedEvent event) -> bool
					{
						KR_CORE_INFO("Event: {0}", event.ToString().c_str());
						return true;
					});
				}
				else if (event == GLFW_CONNECTED)
				{
					ControllerDeviceConnectedEvent eve(cID, event);

					EventDispatcher dispatcher(eve);
					dispatcher.Dispatch<ControllerDeviceConnectedEvent>([](ControllerDeviceConnectedEvent event) -> bool
					{
						KR_CORE_INFO("Event: {0}", event.ToString().c_str());
						return true;
					});
				}
			});
		}
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

	bool LinuxInput::IsMouseButtonReleasedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);

		return state == GLFW_RELEASE;
	}

	bool LinuxInput::IsControllerButtonPressedImpl(int button, int cID)
	{
		// Assuming glfw api
		GLFWgamepadstate cState;

		std::shared_ptr<ControllerDevice> cDevice;

		for (const auto& elem : m_ControllerDevices)
		{
			if (elem->GetControllerID() == cID)
			{
				cDevice = elem;
				break;
			}
			else
			{
				KR_CORE_WARN("Can't find Controller Device corresponding to controller ID: {0}", cID);
				continue;
			}
		}

		if (cDevice && glfwGetGamepadState(cDevice->GetControllerID(), &cState) == GLFW_TRUE)
		{
			if (cState.buttons[button])
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		return false;
	}

	float LinuxInput::ControllerAxisPivotValImpl(int axis, int cID)
	{
		// Assuming glfw api
		GLFWgamepadstate cState;

		std::shared_ptr<ControllerDevice> cDevice;

		for (const auto& elem : m_ControllerDevices)
		{
			if (elem->GetControllerID() == cID)
			{
				cDevice = elem;
				break;
			}
			else
			{
				KR_CORE_WARN("Can't find Controller Device corresponding to controller ID: {0}", cID);
				continue;
			}
		}

		if (cDevice && glfwGetGamepadState(cDevice->GetControllerID(), &cState) == GLFW_TRUE)
		{
			//KR_CORE_INFO("Axis status: {0}", cState.axes[axis]);
			return cState.axes[axis];
		}

		return 0.f;
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
