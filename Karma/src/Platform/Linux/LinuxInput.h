#pragma once

#include "Karma/Input.h"

namespace Karma
{

	struct LinuxInputData : InputData
	{
		LinuxInputData() {}
		Input::EventCallbackFn EventCallback;
	};

	class KARMA_API LinuxInput : public Input
	{
	public:
		LinuxInput();

		void SetEventCallback(const EventCallbackFn& callback, std::shared_ptr<Window> window) override;
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual bool IsMouseButtonReleasedImpl(int button) override;
		virtual bool IsControllerButtonPressedImpl(int button, int cID) override;
		virtual float ControllerAxisPivotValImpl(int axis, int cID) override;

		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;

		// Need to hook GLFW specific calls

		LinuxInputData m_Data;
	};

}
