/**
 * @file MouseEvent.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the classes concerned with the movement and activity of mouse.
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
	 * @brief Event triggered on mouse movement (harkat)
	 */
	class KARMA_API MouseMovedEvent : public Event
	{
	public:
		/**
		 * @brief Default constructor
		 *
		 * @since Karma 1.0.0
		 */
		MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y)
		{
		}

		/**
		 * @brief Getter for x coordinate
		 *
		 * @since Karma 1.0.0
		 */
		inline float GetX() const { return m_MouseX; }

		/**
		 * @brief Getter for y coordinate
		 *
		 * @since Karma 1.0.0
		 */
		inline float GetY() const { return m_MouseY; }

		/**
		 * @brief String representation of event
		 *
		 * @since Karma 1.0.0
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX, m_MouseY;
	};

	/**
	 * @brief Event triggered on scrolling of mouse
	 */
	class KARMA_API MouseScrolledEvent : public Event
	{
	public:
		/**
		 * @brief Default constructor
		 *
		 * @since Karma 1.0.0
		 */
		MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset),
			m_YOffset(yOffset)
		{
		}

		/**
		 * @brief Getter for x offset
		 *
		 * @since Karma 1.0.0
		 */
		inline float GetXOffset() const { return m_XOffset; }

		/**
		 * @brief Getter for y offset
		 *
		 * @since Karma 1.0.0
		 */
		inline float GetYOffset() const { return m_YOffset; }

		/**
		 * @brief String representation of scrolling event
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_XOffset, m_YOffset;
	};

	/**
	 * @brief Base class for mouse button press and release events
	 */
	class KARMA_API MouseButtonEvent : public Event
	{
	public:
		/**
		 * @brief Getter for mouse button
		 *
		 * @since Karma 1.0.0
		 */
		inline int GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	protected:
		MouseButtonEvent(int button) : m_Button(button)
		{
		}
		int m_Button;
	};

	/**
	 * @brief Event triggered when the mouse button is pressed
	 */
	class KARMA_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param button						GLFW defined mouse button, signifying the button pressed
		 * @see https://www.glfw.org/docs/3.3/group__buttons.html
		 *
		 * @since Karma 1.0.0
		 */
		MouseButtonPressedEvent(int button) : MouseButtonEvent(button)
		{
		}

		/**
		 * @brief String representation of the mouse button press event
		 *
		 * @since Karma 1.0.0
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	/**
	 * @brief Event triggered when the mouse button is released
	 */
	class KARMA_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param button						GLFW defined mouse button, signifying the button released
		 * @see https://www.glfw.org/docs/3.3/group__buttons.html
		 *
		 * @since Karma 1.0.0
		 */
		MouseButtonReleasedEvent(int button) : MouseButtonEvent(button)
		{
		}

		/**
		 * @brief String representation of the event
		 *
		 * @since Karma 1.0.0
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}
