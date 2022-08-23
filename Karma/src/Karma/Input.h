#pragma once

#include "krpch.h"

#include "Karma/Log.h"
#include "Karma/Events/Event.h"
#include "Karma/Window.h"
#include "Karma/KarmaUtilities.h"

/*
	Input system need complete revamp in the sense to provide enough context for input maping
	and being able to work with specialized controller devices with minimal latency.
*/


namespace Karma
{
    enum class InputRegisteringAPI
	{
		None = 0,
        GlfwInput
	};

	struct InputData
	{
	};

	class KARMA_API GameAction
	{
	public:
		GameAction();

		// Getters
		const std::string& GetGAName() const { return m_ActionName; }
		const std::shared_ptr<void>& GetClassAction() const { return m_ClassAction; }
	private:
		// The class/object pointer, imparting the action
		std::shared_ptr<void> m_ClassAction;
		std::string m_ActionName;
	};

	class KARMA_API Button
	{
	public:
		Button();

		// Getters
		const int& GetButtonID() const { return m_ButtonID; }
	private:
		int m_ButtonID;

	};

	class KARMA_API ControllerDevice
	{
	public:
		ControllerDevice(int cID, const std::string& dName);

		void AddInputMapping();

		// Getters
		const int& GetControllerID() const { return m_ControllerID; }
		const std::string& GetDeviceName() const { return m_DeviceName; }

	private:
		std::unordered_map<std::shared_ptr<Button>, std::shared_ptr<GameAction>> m_InputMapping;
		int m_ControllerID;
		std::string m_DeviceName;
		bool m_IsGamePad;
	};

	class KARMA_API Input
	{
	public:
		Input(InputData& inputDatRef);
		~Input();

		inline static bool IsKeyPressed(int keycode)
		{
			return s_Instance->IsKeyPressedImpl(keycode);
		}

		inline static bool IsMouseButtonPressed(int button)
		{
			return s_Instance->IsMouseButtonPressedImpl(button);
		}

		inline static bool IsMouseButtonReleased(int button)
		{
			return s_Instance->IsMouseButtonReleasedImpl(button);
		}

		inline static bool IsControllerButtonPressed(int button, int cID)
		{
			return s_Instance->IsControllerButtonPressedImpl(button, cID);
		}

		inline static float ControllerAxisPivotVal(int axis, int cID)
		{
			return s_Instance->ControllerAxisPivotValImpl(axis, cID);
		}

		inline static std::pair<float, float>GetMousePosition()
		{
			return s_Instance->GetMousePositionImpl();
		}

		inline static float GetMouseX()
		{
			return s_Instance->GetMouseXImpl();
		}

		inline static float GetMouseY()
		{
			return s_Instance->GetMouseYImpl();
		}

		inline static InputRegisteringAPI GetAPI() { return s_InputAPI; }

		inline static void DeInit()
		{
			KR_CORE_INFO("Shutting down input system");
		}

		static void Init();

		static std::shared_ptr<Input> GetInputInstance() { return s_Instance; }

		using EventCallbackFn = std::function<void(Event&)>;
		virtual void SetEventCallback(const EventCallbackFn& callback, Window* window) = 0;

		void SetGamepadMapping();

		// Debug purposes
		static void DisplayControllerDevices();

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual bool IsMouseButtonReleasedImpl(int button) = 0;
		virtual bool IsControllerButtonPressedImpl(int button, int cID) = 0;
		virtual float ControllerAxisPivotValImpl(int axis, int cID) = 0;

		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

		// GLFW specific
		static void SetConnectedJoySticks();

		// Getters
		const std::list<std::shared_ptr<ControllerDevice>>& GetControllerDevices() const { return m_ControllerDevices; }

		void AddControllerDevice(std::shared_ptr<ControllerDevice> device);

	private:
		static InputRegisteringAPI s_InputAPI;
		static std::shared_ptr<Input> s_Instance;

	protected:
		static std::list<std::shared_ptr<ControllerDevice>> m_ControllerDevices;
	};
}
