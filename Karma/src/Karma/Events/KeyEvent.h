/**
 * @file KeyEvent.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the classes concerned with activity of keyboard.
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
	 * @brief Base class for keyboard event
	 */
	class KARMA_API KeyEvent : public Event
	{
	public:
		/**
		 * @brief Getter for key code of the keyboard event
		 *
		 * @since Karma 1.0.0
		 */
		inline int GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
		
	protected:
		/**
		 * @brief A constructor
		 *
		 * @param keycode						GLFW defined key code for the event
		 * @see https://www.glfw.org/docs/3.3/group__keys.html
		 *
		 * @since Karma 1.0.0
		 */
		KeyEvent(int keycode) : m_KeyCode(keycode)
		{
		}

		int m_KeyCode;
	};

	/**
	 * @brief Event triggered when the keyboard key is pressed
	 */
	class KARMA_API KeyPressedEvent : public KeyEvent
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param keycode						GLFW defined key code for the key press event
		 * @param repeatCount					Counter for key repeatition
		 *
		 * @see https://www.glfw.org/docs/3.3/group__keys.html
		 * @since Karma 1.0.0
		 */
		KeyPressedEvent(int keycode, int repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount)
		{
		}

		/**
		 * @brief Getter for key repeat counter
		 *
		 * @since Karma 1.0.0
		 */
		inline int GetRepeatCount() const { return m_RepeatCount; }

		/**
		 * @brief String representation of the event
		 *
		 * @since Karma 1.0.0
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " ( " << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	/**
	 * @brief Event triggered when the key is released
	 */
	class KARMA_API KeyReleasedEvent : public KeyEvent
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param keycode						GLFW defined key code
		 * @see https://www.glfw.org/docs/3.3/group__keys.html
		 *
		 * @since Karma 1.0.0
		 */
		KeyReleasedEvent(int keycode) : KeyEvent(keycode)
		{
		}

		/**
		 * @brief String representation of th event
		 *
		 * @since Karma 1.0.0
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	/**
	 * @brief Event triggered when keyboard key is used in typing
	 */
	class KARMA_API KeyTypedEvent : public KeyEvent
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param keycode						GLFW defined key code
		 * @see https://www.glfw.org/docs/3.3/group__keys.html
		 *
		 * @since Karma 1.0.0
		 */
		KeyTypedEvent(int keycode)
			: KeyEvent(keycode)
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
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}
