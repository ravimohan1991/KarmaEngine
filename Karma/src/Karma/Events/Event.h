/**
 * @file Event.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the base class Event for Karma's events.
 * @version 1.0
 * @date November 28, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

namespace Karma
{
	/**
	 * @brief Collection of events used by the Engine
	 */
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		GameControllerConnected, GameControllerDisconnected
	};

	/**
	 * @brief Classification of the above events
	 */
	enum EventCategory
	{
		/**
		 * @brief Events of the unknown category
		 */
		None = 0,

		/**
		 * @brief Events belonging to application specific processes (resizing and movement for instance)
		 */
		EventCategoryApplication				= BIT(0),

		/**
		 * @brief Events belonging to game inputs. %Input device examples include keyboard, mouse, and all that
		 */
		EventCategoryInput						= BIT(1),

		/**
		 * @brief Events belonging to game inputs from keyboard (keyboard buttons pressing for instance)
		 */
		EventCategoryKeyboard					= BIT(2),

		/**
		 * @brief Events belonging to game movement inputs from mouse
		 */
		EventCategoryMouse						= BIT(3),

		/**
		 * @brief Events belonging to game button inputs from mouse (mouse buttons pressing for instance)
		 */
		EventCategoryMouseButton				= BIT(4),

		/**
		 * @brief Events belonging to game controller devices' input.
		 */
		EventCategoryGameControllerDevice		= BIT(5)
	};

/**
 * @brief A macro for a template for Event based classes
 *
 * @since Karma 1.0.0
 */
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

/**
 * @brief Macro for the routine category flags
 *
 * @since Karma 1.0.0
 */
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	/**
	 * @brief The base class of all the events for Karma
	 */
	class KARMA_API Event
	{
		/**
		 * @brief An event dispatching class
		 *
		 * @since Karma 1.0.0
		 */
		friend class EventDispatcher;
	public:
		/**
		 * @brief Getter for event name (type)
		 *
		 * @since Karma 1.0.0
		 */
		virtual EventType GetEventType() const = 0;

		/**
		 * @brief Getter for event type
		 *
		 * @see EVENT_CLASS_TYPE
		 * @since Karma 1.0.0
		 */
		virtual const char* GetName() const = 0;

		/**
		 * @brief Getter for category flags (enum EventCategory)
		 *
		 * @since Karma 1.0.0
		 */
		virtual int GetCategoryFlags() const = 0;

		/**
		 * @brief String representation of the event
		 *
		 * @since Karma 1.0.0
		 */
		virtual std::string ToString() const { return GetName(); }

		/**
		 * @brief Routine to see if the category of the Event is (or a part of) given category
		 *
		 * @since Karma 1.0.0
		 */
		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}

		/**
		 * @brief Getter for the m_Handled
		 *
		 * @since Karma 1.0.0
		 */
		inline bool IsHandled() const
		{
			return m_Handled;
		}

		/**
		 * @brief Getter for the pointer to the object
		 *
		 * @since Karma 1.0.0
		 */
		inline Event* GetObjPointer() { return this; }

	protected:
		// If an event has been handled or not. To implement blocking
		bool m_Handled = false;
	};

	/**
	 * @brief Class for dispatching events
	 */
	class EventDispatcher
	{
		/**
		 * @brief An acronym for function pointer
		 *
		 * @since Karma 1.0.0
		 */
		template<typename T>
		using EventFn = std::function<bool(T&)>;

	public:
		/**
		 * @brief A constructor
		 *
		 * @param event						Reference to an Event
		 * @since Karma 1.0.0
		 */
		EventDispatcher(Event& event) : m_Event(event)
		{
		}

		/**
		 * @brief Routine for dispatching Events
		 *
		 * For a demonstration see the line
		 * @code{.cpp}
		 * dispatcher.Dispatch<MouseButtonPressedEvent>(KR_BIND_EVENT_FN(KarmaGuiLayer::OnMouseButtonPressedEvent));
		 * @endcode
		 *
		 * where KR_BIND_EVENT_FN is defined like so
		 * @code{.cpp}
		 * #define KR_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
		 * @endcode
		 *
		 * Hence, we see that the dispatch function, usually called in OnEvent(), allows the execution of the bounded function (EventFn) with the supplied Event.
		 *
		 * A way to read dispatcher call is like so
		 *
		 * "I am present in so class with such routine (fn), hence I am dispatching supplied Event with a bounded function (KR_BIND_EVENT_FN(fn)) to be executed"
		 *
		 * @note GLFW calls the Application::OnEvent which further calls the OnEvent function in the subsequent layers and,
		 * depending on m_Handled, the depth of the call chain (measured in the sense of for-loop, upon the ordered layers) is determined.
		 *
		 * @see Application::OnEvent(Event& e)
		 *
		 * @since Karma 1.0.0
		 */
		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	/*
	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
	*/
}
