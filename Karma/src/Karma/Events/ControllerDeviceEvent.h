#pragma once

#include "Karma/Core.h"
#include "Event.h"
#include <sstream>

namespace Karma
{
	class KARMA_API ControllerDeviceConnectedEvent : public Event
	{
	public:
		ControllerDeviceConnectedEvent(int cID, int status) : m_ControllerID(cID), m_Status(status)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ControllerDeviceConnectedEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(GameControllerConnected)
		EVENT_CLASS_CATEGORY(EventCategoryGameControllerDevice)
	private:
		int m_ControllerID;
		int m_Status;
	};

	class KARMA_API ControllerDeviceDisconnectedEvent : public Event
	{
	public:
		ControllerDeviceDisconnectedEvent(int cID, int status) : m_ControllerID(cID), m_Status(status)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ControllerDeviceDisconnectedEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(GameControllerConnected)
		EVENT_CLASS_CATEGORY(EventCategoryGameControllerDevice)
	private:
		int m_ControllerID;
		int m_Status;
	};
}