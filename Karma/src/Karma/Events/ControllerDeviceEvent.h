/**
 * @file ControllerDeviceEvent.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the classes concerned with activity of game controller device.
 * @version 1.0
 * @date May 1, 2022
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Event.h"

namespace Karma
{
	/**
	 * @brief Event triggered when the constroller device is connected
	 */
	class KARMA_API ControllerDeviceConnectedEvent : public Event
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param cID					The controller device ID, defined in GLFW
		 * @param status				Current status of the controller device, defined in GLFW
		 * 									#define GLFW_CONNECTED              0x00040001
		 *									#define GLFW_DISCONNECTED           0x00040002
		 *
		 * @see https://www.glfw.org/docs/3.3/group__joysticks.html
		 * @since Karma 1.0.0
		 */
		ControllerDeviceConnectedEvent(int cID, int status) : m_ControllerID(cID), m_Status(status)
		{
		}

		/**
		 * @brief String representation of the event
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ControllerDeviceConnectedEvent: joystick " << m_ControllerID;

			return ss.str();
		}

		EVENT_CLASS_TYPE(GameControllerConnected)
		EVENT_CLASS_CATEGORY(EventCategoryGameControllerDevice)
	private:
		int m_ControllerID;
		int m_Status;
	};

	/**
	 * @brief Event triggered when controller device is disconnected
	 */
	class KARMA_API ControllerDeviceDisconnectedEvent : public Event
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param cID					The controller device ID, defined in GLFW
		 * @param status				Current status of the controller device, defined in GLFW
		 * 									#define GLFW_CONNECTED              0x00040001
		 *									#define GLFW_DISCONNECTED           0x00040002
		 *
		 * @see https://www.glfw.org/docs/3.3/group__joysticks.html
		 * @since Karma 1.0.0
		 */
		ControllerDeviceDisconnectedEvent(int cID, int status) : m_ControllerID(cID), m_Status(status)
		{
		}

		/**
		 * @brief String representation of the event
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ControllerDeviceDisconnectedEvent: joystick " << m_ControllerID;

			return ss.str();
		}

		EVENT_CLASS_TYPE(GameControllerConnected)
		EVENT_CLASS_CATEGORY(EventCategoryGameControllerDevice)
	private:
		int m_ControllerID;
		int m_Status;
	};
}
