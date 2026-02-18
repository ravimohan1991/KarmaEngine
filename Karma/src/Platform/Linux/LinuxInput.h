/**
 * @file LinuxInput.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains LinuxInput class and relevant data structures.
 * @version 1.0
 * @date December 13, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Karma/Input.h"

namespace Karma
{
	/**
	 * @brief LinuxInput specific data structure
	 */
	struct LinuxInputData : InputData
	{
		/**
		 * @brief A Constructor
		 *
		 * @since Karma 1.0.0
		 */
		LinuxInputData() {}

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
	 * @brief Input class for Linux based OS
	 */
	class KARMA_API LinuxInput : public Input
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		LinuxInput();

		/**
		 * @brief Set the event callback for the Linux Input
		 *
		 * @param callback						The function to be called when Input event is receieved from GLFW.
		 * 										Usually Application::OnEvent(), which dispatches the Events, is fed into as lambda for instance in glfwSetJoystickCallback
		 * @param window						The handle of GLFW window
		 * @since Karma 1.0.0
		 */
		void SetEventCallback(const EventCallbackFn& callback, Window* window) override;
	protected:

		/**
		 * @brief Linux implementation of Linux IsKeyPressed function
		 *
		 * @param keycode						GLFW defined code for keys, for instance GLFW_KEY_SPACE macro
		 * 										https://www.glfw.org/docs/3.3/group__keys.html
		 * @since Karma 1.0.0
		 */
		virtual bool IsKeyPressedImpl(int keycode) override;

		/**
		 * @brief Linux implementation of Linux IsMouseButtonPressed function
		 *
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 * @since Karma 1.0.0
		 */
		virtual bool IsMouseButtonPressedImpl(int button) override;

		/**
		 * @brief Linux implementation of IsMouseButtonReleased function
		 *
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 * @since Karma 1.0.0
		 */
		virtual bool IsMouseButtonReleasedImpl(int button) override;

		/**
		 * @brief Linux implementation of IsControllerButtonPressed function
		 *
		 * @param button						The integer corresponding to gamepad controller button.
		 * 										For instance GLFW_GAMEPAD_BUTTON_A macro.
		 * @param cID							The controller device ID, defined in GLFW
		 *
		 * @since Karma 1.0.0
		 */
		virtual bool IsControllerButtonPressedImpl(int button, int cID) override;

		/**
		 * @brief Linux implementation of ControllerAxisPivotVal function
		 *
		 * @param axis						The horizontal/vertical directional deflection of the axis (GLFW axes).
		 * 									GLFW_GAMEPAD_AXIS_LEFT_Y is an example.
		 * @param cID						The ID of the attached controller, defined in GLFW
		 *
		 * @since Karma 1.0.0
		 */
		virtual float ControllerAxisPivotValImpl(int axis, int cID) override;

		/**
		 * @brief Linux implementation of GetMousePosition function
		 *
		 * @since Karma 1.0.0
		 */
		virtual std::pair<float, float> GetMousePositionImpl() override;

		/**
		 * @brief Linux implementation of GetMouseX function
		 *
		 * @since Karma 1.0.0
		 */
		virtual float GetMouseXImpl() override;

		/**
		 * @brief Linux implementation of GetMouseY function
		 *
		 * @since Karma 1.0.0
		 */
		virtual float GetMouseYImpl() override;

		// Need to hook GLFW specific calls
		LinuxInputData m_Data;
	};
}
