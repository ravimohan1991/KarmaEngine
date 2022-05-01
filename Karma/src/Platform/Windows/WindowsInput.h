#pragma once

#include "Karma/Input.h"
#include "Karma/Events/ControllerDeviceEvent.h"

namespace Karma
{
	struct WindowsInputData : InputData
	{
		WindowsInputData() {}
		Input::EventCallbackFn EventCallback;
	};

	class KARMA_API WindowsInput : public Input
	{
	public:
		WindowsInput();

		void SetEventCallback(const EventCallbackFn& callback, std::shared_ptr<Window> window) override;

	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual bool IsMouseButtonReleasedImpl(int button) override;
		virtual bool IsControllerButtonPressedImpl(int button, int cID) override;
		virtual float ControllerAxisPivotMagImpl(int axis, int cID) override;

		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;

		void OnControllerDeviceConnected(ControllerDeviceConnectedEvent& e);
		void OnControllerDeviceDisconnected(ControllerDeviceDisconnectedEvent& e);

		WindowsInputData m_Data;
	};

}