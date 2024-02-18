/**
 * @file Input.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains input class and relevant data structures.
 * @version 1.0
 * @date December 12, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Log.h"
#include "Karma/Events/Event.h"
#include "Karma/Window.h"
#include "Karma/KarmaUtilities.h"

/*
	Input system need complete revamp in the sense to provide enough context for input maping
	and being able to work with specialized controller devices with minimal latency.

	Partial progress has been made towards that direction.
*/


namespace Karma
{
	/**
	 * @brief The input API being used by Karma
	 *
	 * @since Karma 1.0.0
	 */
	enum class InputRegisteringAPI
	{
		/**
		 * @brief No input
		 */
		None = 0,
		/**
		 * @brief Using the GLFW library
		 */
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

	/**
	 * @brief Basic input class for game devices
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API Input
	{
	public:
		/**
		 * @brief A constructor for initializing the Input class
		 *
		 * Does the following stuff
		 * 1. Set the gamepad mapping based on a database (../Resources/Misc/GameControllerDB.txt)
		 * 	The mapping is done by taking XBox controller as a reference. For the need of such mapping
		 *
		 * 	please see https://www.glfw.org/docs/3.3/input_guide.html#gamepad_mapping
		 *
		 * 	@code{}
		 * 	The joystick functions provide unlabeled axes, buttons and hats, with no indication of where they are located on the device.
		 * 	Their order may also vary between platforms even with the same device.
		 *
		 * 	To solve this problem the SDL community crowdsourced the SDL_GameControllerDB project, a database of mappings from many
		 * 	different devices to an Xbox-like gamepad.
		 * 	@endcode
		 *
		 * 2. Create a list of connected joysticks (gamepads)
		 * 3. Setting a callback function containing structure pointer, for all connected joysticks (gamepads)
		 * 4. All the above steps are taken with API modularity (assuming GLFW API for input)
		 *
		 * @since Karma 1.0.0
		 */
		Input(InputData& inputDatRef);

		/**
		 * @brief Destructor
		 *
		 * @since Karma 1.0.0
		 */
		~Input();

		/**
		 * @brief Polling function for inquiry of key press
		 *
		 * @param keycode						GLFW defined code for keys, for instance GLFW_KEY_SPACE macro
		 * 										https://www.glfw.org/docs/3.3/group__keys.html
		 *
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		inline static bool IsKeyPressed(int keycode)
		{
			return s_Instance->IsKeyPressedImpl(keycode);
		}

		/**
		 * @brief Polling function for inquiry of mouse button press
		 *
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 *
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		inline static bool IsMouseButtonPressed(int button)
		{
			return s_Instance->IsMouseButtonPressedImpl(button);
		}

		/**
		 * @brief Polling function for the inquiry of mouse button release
		 *
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 *
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		inline static bool IsMouseButtonReleased(int button)
		{
			return s_Instance->IsMouseButtonReleasedImpl(button);
		}

		/**
		 * @brief Polling function for the inquiry of game controller button pressed action
		 *
		 * @param button						The integer corresponding to gamepad controller button
		 * 										For instance GLFW_GAMEPAD_BUTTON_A macro
		 * @param cID							The controller device ID, defined in GLFW
		 *
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		inline static bool IsControllerButtonPressed(int button, int cID)
		{
			return s_Instance->IsControllerButtonPressedImpl(button, cID);
		}

		/**
		 * @brief Polling function for the inquiry of the Axis pivot value (analog stick deflection for instance)
		 *
		 * @param axis						The horizontal/vertical directional deflection of the axis (GLFW axes).
		 * 									GLFW_GAMEPAD_AXIS_LEFT_Y is an example
		 * @param cID						The ID of the attached controller, defined in GLFW
		 *
		 * @return float value rage -1.0f to 1.0f
		 *
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		inline static float ControllerAxisPivotVal(int axis, int cID)
		{
			return s_Instance->ControllerAxisPivotValImpl(axis, cID);
		}

		/**
		 * @brief Polling function for the inquiry of mouse position (abscissa, ordinate)
		 *
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		inline static std::pair<float, float>GetMousePosition()
		{
			return s_Instance->GetMousePositionImpl();
		}

		/**
		 * @brief Polling function for the inquiry of abscissa
		 *
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		inline static float GetMouseX()
		{
			return s_Instance->GetMouseXImpl();
		}

		/**
		 * @brief Polling function for the inquiry of ordinate
		 *
		 * @note GLFW API is used
		 * @since Karma 1.0.0
		 */
		inline static float GetMouseY()
		{
			return s_Instance->GetMouseYImpl();
		}

		/**
		 * @brief Getter for API (enum) in use for input
		 *
		 * @see InputRegisteringAPI
		 * @since Karma 1.0.0
		 */
		inline static InputRegisteringAPI GetAPI() { return s_InputAPI; }

		/**
		 * @brief Deinitialize the Input system
		 *
		 * @since Karma 1.0.0
		 */
		inline static void DeInit()
		{
			KR_CORE_INFO("Shutting down input system");
		}

		/**
		 * @brief Initialize the Input system for Karma
		 *
		 * Based upon the platform, the appropriate Input instance is created.
		 *
		 * @note ATM the constructors are empty
		 *
		 * @since Karma 1.0.0
		 */
		static void Init();

		/**
		 * @brief Getter for the Input instance
		 *
		 * @since Karma 1.0.0
		 */
		static std::shared_ptr<Input> GetInputInstance() { return s_Instance; }

		using EventCallbackFn = std::function<void(Event&)>;

		/**
		 * @brief Set the event callback for the Input
		 *
		 * @param callback						The function to be called when Input event is receieved from GLFW
		 * 										Usually Application::OnEvent(), which dispatches the Events, is fed into as lambda for instance in glfwSetJoystickCallback
		 * @param window						The handle of GLFW window
		 *
		 * @since Karma 1.0.0
		 */
		virtual void SetEventCallback(const EventCallbackFn& callback, Window* window) = 0;

		/**
		 * @brief Set the gamepad mapping based on a database (../Resources/Misc/GameControllerDB.txt)
		 * 	The mapping is done by taking XBox controller as a reference. For the need of such mapping please see https://www.glfw.org/docs/3.3/input_guide.html#gamepad_mapping
		 *
		 * 	@code{}
		 * 	The joystick functions provide unlabeled axes, buttons and hats, with no indication of where they are located on the device.
		 * 	Their order may also vary between platforms even with the same device.
		 *
		 * 	To solve this problem the SDL community crowdsourced the SDL_GameControllerDB project, a database of mappings from many
		 * 	different devices to an Xbox-like gamepad.
		 * 	@endcode
		 *
		 * The code is like so
		 * 	\code{.cpp}
		 * 		std::string testString = KarmaUtilities::ReadFileToSpitString("../Resources/Misc/GameControllerDB.txt");
		 *
		 * 		const char* mappings = testString.c_str();
		 * 		glfwUpdateGamepadMappings(mappings);
		 * 	\endcode
		 *
		 * @since Karma 1.0.0
		 */
		void SetGamepadMapping();

		// Debug purposes
		/**
		 * @brief For debugging purposes, a function to print out the number of connected devices
		 *
		 * @since Karma 1.0.0
		 */
		static void DisplayControllerDevices();

	protected:
		/**
		 * @brief Declaration of IsKeyPressed function implementation
		 *
		 * @param keycode						GLFW defined code for keys, for instance GLFW_KEY_SPACE macro
		 * 										https://www.glfw.org/docs/3.3/group__keys.html
		 * @since Karma 1.0.0
		 */
		virtual bool IsKeyPressedImpl(int keycode) = 0;

		/**
		 * @brief Declaration of IsMouseButtonPressed function implementation
		 *
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 * @since Karma 1.0.0
		 */
		virtual bool IsMouseButtonPressedImpl(int button) = 0;

		/**
		 * @brief Declaration of IsMouseButtonReleased function implementation
		 *
		 * @param button						The integer corresponding to mouse button.
		 * 										For instance GLFW_MOUSE_BUTTON_1 macro
		 * @since Karma 1.0.0
		 */
		virtual bool IsMouseButtonReleasedImpl(int button) = 0;

		/**
		 * @brief Declaration of IsControllerButtonPressed function implementation
		 *
		 * @param button						The integer corresponding to gamepad controller button
		 * 										For instance GLFW_GAMEPAD_BUTTON_A macro
		 * @param cID							The controller device ID, defined in GLFW
		 *
		 * @since Karma 1.0.0
		 */
		virtual bool IsControllerButtonPressedImpl(int button, int cID) = 0;

		/**
		 * @brief Declaration of ControllerAxisPivotVal function implementation
		 *
		 * @param axis						The horizontal/vertical directional deflection of the axis (GLFW axes).
		 * 									GLFW_GAMEPAD_AXIS_LEFT_Y is an example
		 * @param cID						The ID of the attached controller, defined in GLFW
		 *
		 * @since Karma 1.0.0
		 */
		virtual float ControllerAxisPivotValImpl(int axis, int cID) = 0;

		/**
		 * @brief Declaration of GetMousePosition function implementation
		 *
		 * @since Karma 1.0.0
		 */
		virtual std::pair<float, float> GetMousePositionImpl() = 0;

		/**
		 * @brief Declaration of GetMouseX function implementation
		 *
		 * @since Karma 1.0.0
		 */
		virtual float GetMouseXImpl() = 0;

		/**
		 * @brief Declaration of GetMouseY function implementation
		 *
		 * @since Karma 1.0.0
		 */
		virtual float GetMouseYImpl() = 0;

		// GLFW specific
		/**
		 * @brief Create a list of connected joysticks (gamepads)
		 *
		 * @since Karma 1.0.0
		 */
		static void SetConnectedJoySticks();

		// Getters
		/**
		 * @brief Getter for connected controller devices
		 *
		 * @since Karma 1.0.0
		 */
		const std::list<std::shared_ptr<ControllerDevice>>& GetControllerDevices() const { return m_ControllerDevices; }

		/**
		 * @brief Add to the cache supplied device
		 *
		 * @param device						The device supplied to the function
		 * @since Karma 1.0.0
		 */
		void AddControllerDevice(std::shared_ptr<ControllerDevice> device);

	private:
		static InputRegisteringAPI s_InputAPI;
		static std::shared_ptr<Input> s_Instance;

	protected:
		static std::list<std::shared_ptr<ControllerDevice>> m_ControllerDevices;
	};
}
