/**
 * @file ApplicationEvent.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the classes concerned with activity of Window.
 * @version 1.0
 * @date November 28, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Event.h"

namespace Karma
{
	/**
	 * @brief Event triggered when Window is resized
	 */
	class KARMA_API WindowResizeEvent : public Event
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param width						The width size of the window after(?) the resizing
		 * @param height					The height size of the window after(?) the resizing
		 *
		 * @note The use of unsigned int
		 * @since Karma 1.0.0
		 */
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height)
		{
		}

		/**
		 * @brief Getter for width size
		 *
		 * @since Karma 1.0.0
		 */
		inline unsigned int GetWidth() const { return m_Width; }

		/**
		 * @brief Getter for height size
		 *
		 * @since Karma 1.0.0
		 */
		inline unsigned int GetHeight() const { return m_Height; }

		/**
		 * @brief String representation of the event
		 *
		 * @since Karma 1.0.0
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		unsigned int m_Width, m_Height;
	};

	/**
	 * @brief Event triggered when window is closed
	 */
	class KARMA_API WindowCloseEvent : public Event
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @note The constructor is empty
		 * @since Karma 1.0.0
		 */
		WindowCloseEvent()
		{
		}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	/**
	 * @brief Event triggered on each game tick (called in game loop)
	 */
	class KARMA_API AppTickEvent : public Event
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		AppTickEvent()
		{
		}

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	/**
	 * @brief Dunno
	 */
	class KARMA_API AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent()
		{
		}

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	/**
	 * @brief Dunno
	 */
	class KARMA_API AppRenderEvent : public Event
	{
	public:
		AppRenderEvent()
		{
		}

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
}
