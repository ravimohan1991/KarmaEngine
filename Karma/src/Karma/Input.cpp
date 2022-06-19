#include "Input.h"
#include "Platform/Windows/WindowsInput.h"
#include "Platform/Linux/LinuxInput.h"
#include "GLFW/glfw3.h"
#include "Karma/KarmaUtilites.h"

namespace Karma
{
    InputRegisteringAPI Input::s_InputAPI = InputRegisteringAPI::GlfwInput;
	std::list<std::shared_ptr<ControllerDevice>> Karma::Input::m_ControllerDevices;
	std::shared_ptr<Karma::Input> Karma::Input::s_Instance = nullptr;// Write a note of this linking stuff. Think what happens without this line. Also ensure singleton pattern.

	ControllerDevice::ControllerDevice(int cID, const std::string& dName) : m_ControllerID(cID), m_DeviceName(dName)
	{
		KR_CORE_INFO("Spawned instance for ControllerDevice: {0}, with id: {1}", dName, cID);
		switch (Input::GetAPI())
		{
            case InputRegisteringAPI::GlfwInput:
			{
				m_IsGamePad = glfwJoystickIsGamepad(cID);
				if (m_IsGamePad)
				{
					KR_CORE_INFO("{0} has a XBox gamepad mapping (a good interpretation!)", dName);
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
            case InputRegisteringAPI::GlfwInput:
			{
				if (glfwInit() == 0)
				{
					KR_CORE_WARN("GLFW can't be initialized. Input system won't support controller device functionality.");
					return;
				}
				SetGamepadMapping();
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

	void Input::SetGamepadMapping()
	{
		std::string testString = KarmaUtilities::ReadFileToSpitString("../Resources/Misc/GameControllerDB.txt");
		
		const char* mappings = testString.c_str();

		glfwUpdateGamepadMappings(mappings);
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
				
				if (glfwJoystickIsGamepad(i) == false)
				{
					KR_CORE_WARN("There is no XBox mapping for the detected device ({0}).  Consequently, Karma can't (and won't) interpret the hardware.  Please check the mapping database", glfwGetJoystickName(i));
					continue;
				}
				else
				{
					joyStick.reset(new ControllerDevice(i, glfwGetGamepadName(i)));
				}
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
		Karma::Input::s_Instance.reset(new WindowsInput());
#endif
#ifdef KR_LINUX_PLATFORM
        s_Instance.reset(new LinuxInput());
#endif
	}
}
