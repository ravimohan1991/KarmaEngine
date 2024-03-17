/**
 * @file MacInput.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains MacInput class and relevant data structures.
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
	 * @brief MacInput specific data structure
	 */
	struct MacInputData : InputData
	{
		/**
		 * @brief A Constructor
		 *
		 * @since Karma 1.0.0
		 */
		MacInputData() {}

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
	 * @brief Input class for MacOS
	 */
	class KARMA_API MacInput : public Input
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		MacInput();

		/**
		 * @brief Set the event callback for MacOS
		 *
		 * @param callback						The function to be called when Input event is receieved from GLFW.
		 * 										Usually Application::OnEvent(), which dispatches the Events, is fed into as lambda for instance in glfwSetJoystickCallback
		 * @param window						The handle of GLFW window
		 *
		 * @since Karma 1.0.0
		 */
		void SetEventCallback(const EventCallbackFn& callback, Window* window) override;

	protected:
		/**
		 * @brief MacOS implementation of Linux IsKeyPressed function
		 *
		 * @param keycode						GLFW defined code for keys, for instance GLFW_KEY_SPACE macro
		 * 										https://www.glfw.org/docs/3.3/group__keys.html
		 * @since Karma 1.0.0
		 */
		virtual bool IsKeyPressedImpl(int keycode) override;

		/**
		 * @brief MacOS implementation of Linux IsMouseButtonPressed function
		 *
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 * @since Karma 1.0.0
		 */
		virtual bool IsMouseButtonPressedImpl(int button) override;

		/**
		 * @brief MacOS implementation of IsMouseButtonReleased function
		 *
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 * @since Karma 1.0.0
		 */
		virtual bool IsMouseButtonReleasedImpl(int button) override;

		/**
		 * @brief MacOS implementation of IsControllerButtonPressed function
		 *
		 * @param button						The integer corresponding to gamepad controller button.
		 * 										For instance GLFW_GAMEPAD_BUTTON_A macro.
		 * @param cID							The controller device ID, defined in GLFW
		 *
		 * @since Karma 1.0.0
		 */
		virtual bool IsControllerButtonPressedImpl(int button, int cID) override;

		/**
		 * @brief MacOS implementation of ControllerAxisPivotVal function
		 *
		 * @param axis						The horizontal/vertical directional deflection of the axis (GLFW axes).
		 * 									GLFW_GAMEPAD_AXIS_LEFT_Y is an example.
		 * @param cID						The ID of the attached controller, defined in GLFW
		 *
		 * @since Karma 1.0.0
		 */
		virtual float ControllerAxisPivotValImpl(int axis, int cID) override;

		/**
		 * @brief MacOS implementation of GetMousePosition function
		 *
		 * @since Karma 1.0.0
		 */
		virtual std::pair<float, float> GetMousePositionImpl() override;

		/**
		 * @brief MacOS implementation of GetMouseX function
		 *
		 * @since Karma 1.0.0
		 */
		virtual float GetMouseXImpl() override;

		/**
		 * @brief MacOS implementation of GetMouseY function
		 *
		 * @since Karma 1.0.0
		 */
		virtual float GetMouseYImpl() override;

		// Need to hook GLFW specific calls

		MacInputData m_Data;
	};

}
