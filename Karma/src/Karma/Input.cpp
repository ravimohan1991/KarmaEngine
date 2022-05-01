#include "Input.h"
#include "Platform/Windows/WindowsInput.h"
#include "GLFW/glfw3.h"

namespace Karma
{
	InputRegisteringAPI Input::s_InputAPI = InputRegisteringAPI::GLFW;
	std::shared_ptr<Input> Input::s_Instance;// Write a note of this linking stuff. Think what happens without this line. Also ensure singleton pattern.
	std::list<std::shared_ptr<ControllerDevice>> Input::m_ControllerDevices;

	ControllerDevice::ControllerDevice(int cID, const std::string& dName) : m_ControllerID(cID), m_DeviceName(dName)
	{
		KR_CORE_INFO("Spawned instance for ControllerDevice: {0}, with id: {1}", dName, cID);
		switch (Input::GetAPI())
		{
			case InputRegisteringAPI::GLFW:
			{
				m_IsGamePad = glfwJoystickIsGamepad(cID);
				if (m_IsGamePad)
				{
					KR_CORE_INFO("{0} has a gamepad mapping", dName);
				}
			}
			break;
			case InputRegisteringAPI::None:
				KR_CORE_WARN("InputRegisteringAPI is set to None. Good Luck!");
				break;
		}
	}

	void ControllerDevice::AddInputMapping()
	{
	}

	void Input::DisplayControllerDevices()
	{
		KR_CORE_INFO("Displaying detected game controller devices:");
		int i = 1;
		for (auto& device : m_ControllerDevices)
		{
			KR_CORE_INFO("{0}. Name: {1}, ID: {2}", i++, device->GetDeviceName(), device->GetControllerID());
		}
	}

	GameAction::GameAction()
	{
	
	}

	Button::Button()
	{
	
	}

	Input::Input(InputData& inputDatRef)
	{
		switch (Input::GetAPI())
		{
			case InputRegisteringAPI::GLFW:
			{
				if (glfwInit() == 0)
				{
					KR_CORE_WARN("GLFW can't be initialized. Input system won't support controller device functionality.");
					return;
				}
				SetConnectedJoySticks();
				for (const auto& elem : m_ControllerDevices)
				{
					glfwSetJoystickUserPointer(elem->GetControllerID(), &inputDatRef);
					KR_CORE_INFO("setting user pointer for {0}", elem->GetControllerID());
				}
			}
			break;
			case InputRegisteringAPI::None:
				KR_CORE_WARN("InputRegisteringAPI is set to None. Good Luck!");
				break;
		}
	}

	Input::~Input()
	{
	}

	void Input::SetConnectedJoySticks()
	{	
		int present = 0;
		
		for (int i = 0; i < GLFW_JOYSTICK_LAST; i++)
		{
			present = glfwJoystickPresent(i);

			if (present == 1)
			{
				std::shared_ptr<ControllerDevice> joyStick;
				joyStick.reset(new ControllerDevice(i, glfwGetGamepadName(i)));
				m_ControllerDevices.push_back(joyStick);
			}
		}
	}

	void Input::AddControllerDevice(std::shared_ptr<ControllerDevice> device)
	{
		m_ControllerDevices.push_back(device);
	}

	void Input::Init()
	{
#ifdef KR_WINDOWS_PLATFORM
		s_Instance.reset(new WindowsInput());
#endif
#ifdef KR_LINUX_PLATFORM
		Input* Input::s_Instance = new LinuxInput();
#endif
	}
}