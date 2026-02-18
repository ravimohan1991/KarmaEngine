/**
 * @file WindowsInput.h
 * @author Ravi Mohan (the_cowboy)
 * @brief Windows specific implementation of the Input class
 * @version 1.0
 * @date December 12, 2020
 * 
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Input.h"
#include "Karma/Events/ControllerDeviceEvent.h"

namespace Karma
{
	/**
	 * @brief Windows specific data structure for Input system
	 * 
	 * @since Karma 1.0.0
	 */
	struct WindowsInputData : InputData
	{
		/**
		 * @brief A constructor
		 * 
		 * @since Karma 1.0.0
		 */
		WindowsInputData() {}
		
		/**
		 * @brief The function to call on detecting Input event from API
		 * 
		 * Usually Application::OnEvent function is used for the callback.
		 * 
		 * @see Application::HookInputSystem
		 * @since Karma 1.0.0
		 */
		Input::EventCallbackFn EventCallback;
	};

	/**
	 * @brief Input class for Windows OS
	 */
	class KARMA_API WindowsInput : public Input
	{
	public:
		/**
		 * @brief A constructor
		 * 
		 * @since Karma 1.0.0
		 */
		WindowsInput();

		/**
		 * @brief Set the event callback for the Windows Input
		 * 
		 * @param callback						The function to be called when Input event is receieved from GLFW.
		 * @param window						The handle of GLFW window
		 * 
		 * @since Karma 1.0.0
		 */
		void SetEventCallback(const EventCallbackFn& callback, Window* window) override;

	protected:

		/**
		 * @brief Windows implementation of IsKeyPressed function
		 * 
		 * @param keycode						GLFW defined code for keys, for instance GLFW_KEY_SPACE macro
		 * 										https://www.glfw.org/docs/3.3/group__keys.html
		 * 
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		virtual bool IsKeyPressedImpl(int keycode) override;
		
		/**
		 * @brief Windows implementation of IsMouseButtonPressed function
		 * 
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 * 
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		virtual bool IsMouseButtonPressedImpl(int button) override;
		
		/**
		 * @brief Windows implementation of IsMouseButtonReleased function
		 * 
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 * 
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		virtual bool IsMouseButtonReleasedImpl(int button) override;
		
		/**
		 * @brief Windows implementation of IsControllerButtonPressed function
		 * 
		 * @param button						The integer corresponding to gamepad controller button
		 * 										For instance GLFW_GAMEPAD_BUTTON_A macro
		 * @param cID							The controller device ID, defined in GLFW
		 * 
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		virtual bool IsControllerButtonPressedImpl(int button, int cID) override;
		
		/**
		 * @brief Windows implementation of ControllerAxisPivotVal function
		 * 
		 * @param axis							The integer corresponding to gamepad controller axis
		 * 										For instance GLFW_GAMEPAD_AXIS_LEFT_X macro
		 * @param cID							The controller device ID, defined in GLFW
		 * 
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		virtual float ControllerAxisPivotValImpl(int axis, int cID) override;

		/**
		 * @brief Windows implementation of GetMousePosition function
		 * 
		 * @return std::pair<float, float>		The X and Y position of the mouse cursor
		 * 
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		virtual std::pair<float, float> GetMousePositionImpl() override;
		
		/**
		 * @brief Windows implementation of GetMouseX function
		 * 
		 * @return float						The X position of the mouse cursor
		 * 
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		virtual float GetMouseXImpl() override;
		
		/**
		 * @brief Windows implementation of GetMouseY function
		 * 
		 * @return float						The Y position of the mouse cursor
		 * 
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		virtual float GetMouseYImpl() override;

		/**
		 * @brief Event handler for Controller Device Connected event
		 * 
		 * @param e								The reference to ControllerDeviceConnectedEvent event object
		 * 
		 * @since Karma 1.0.0
		 */
		void OnControllerDeviceConnected(ControllerDeviceConnectedEvent& e);
		
		/**
		 * @brief Event handler for Controller Device Disconnected event
		 * 
		 * @param e								The reference to ControllerDeviceDisconnectedEvent event object
		 * 
		 * @since Karma 1.0.0
		 */
		void OnControllerDeviceDisconnected(ControllerDeviceDisconnectedEvent& e);

		WindowsInputData m_Data;
	};

}
