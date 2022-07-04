#include "MacInput.h"
#include "GLFW/glfw3.h"
#include "Karma/Application.h"

namespace Karma
{
	MacInput::MacInput() : m_Data(), Input(m_Data)
	{
	}

	void MacInput::SetEventCallback(const EventCallbackFn& callback, std::shared_ptr<Window> window)
	{
		m_Data.EventCallback = callback;

		//WindowsInputData data = *static_cast<WindowsInputData*>(glfwGetJoystickUserPointer(0));
		//data.EventCallback;

		// We need to send event information  to Application class, somehow

		if (Input::GetAPI() == InputRegisteringAPI::GlfwInput)
		{
			glfwSetJoystickCallback([](int cID, int event)
			{
				if (event == GLFW_DISCONNECTED)
				{
			//WindowsInputData& data = *static_cast<WindowsInputData*>(glfwGetJoystickUserPointer(cID)); returns null after disconnection so no use

					ControllerDeviceDisconnectedEvent eve(cID, event);
					//data.EventCallback(eve);

					EventDispatcher dispatcher(eve);
			dispatcher.Dispatch<ControllerDeviceDisconnectedEvent>([](ControllerDeviceDisconnectedEvent event) -> bool
					{
						KR_CORE_INFO("Event: {0}", event.ToString().c_str());
						return true;
					});
				}
				else if (event == GLFW_CONNECTED)
				{
			//void* test = glfwGetJoystickUserPointer(cID);// how to call application function when pointer is not set?

					//WindowsInputData& data = *static_cast<WindowsInputData*>(test);

					ControllerDeviceConnectedEvent eve(cID, event);
					//data.EventCallback(eve);

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

	bool MacInput::IsControllerButtonPressedImpl(int button, int cID)
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

	bool MacInput::IsMouseButtonReleasedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);

		return state == GLFW_RELEASE;
	}

	float MacInput::ControllerAxisPivotValImpl(int axis, int cID)
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

	bool MacInput::IsKeyPressedImpl(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool MacInput::IsMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);

		return state == GLFW_PRESS;
	}

	std::pair<float, float> MacInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float MacInput::GetMouseXImpl()
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

	float MacInput::GetMouseYImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return (float)ypos;
	}
}
