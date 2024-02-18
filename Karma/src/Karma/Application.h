/**
 * @file Application.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the main Application class for Editor + Game.
 * @version 1.0
 * @date November 27, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Window.h"
#include "Karma/Events/ApplicationEvent.h"
#include "Karma/Events/ControllerDeviceEvent.h"
#include "Karma/LayerStack.h"
#include "Karma/KarmaGui/KarmaGuiLayer.h"
#include "Karma/Input.h"
#include "Scene.h"
#include "Core/TrueCore/KarmaSmriti.h"

namespace Karma
{
	/**
	 * @brief The type of platform application is running on.
	 */
	enum class RunningPlatform
	{
		/**
		 * @brief For Linux based OS support
		 */
		Linux = 0,

		/**
		 * @brief For MacOS (a Unix based OS) support
		 */
		Mac,

		/**
		 * @brief For Windows
		 */
		Windows
	};

	/**
	 * @brief The class for Game + Editor in application form
	 */
	class KARMA_API Application
	{
	public:
		/**
		 * @brief A constructor for Application
		 *
		 * Does the following steps
		 * 	- A check for ensuring only one instance is present
		 * 	- Creation of a Window (depending upon the platform)
		 * 	- Declaration of Layer stack
		 * 	- Creation of KarmaGui Layer and a push into the stack
		 *
		 * @since Karma 1.0.0
		 */
		Application();
		
		/**
		 * @brief Destructor
		 *
		 *	- Clearence of the memory (and relevant) resources
		 *	- Clearence of rendering data
		 *	- Deletion of Layer stack
		 *	- Deletion of Application window
		 *
		 * @since Karma 1.0.0
		 */
		~Application();

		/**
		 * @brief Routine containing the main application loop
		 *
		 * The purpose of the loop is multifold
		 * 	- Tracking the clock time gap between consecutive loop iterations (thus compute the deltatime)
		 * 	- Make GEngine (instance of KEngine class) tick
		 * 	- Update all the layers of stack
		 * 	- Update all the layers' UI rendering (KarmaGui's sequence)
		 * 	- Window update
		 *
		 * @since Karma 1.0.0
		 */
		void Run();

		/**
		 * @brief For dispatching events and trickling down the call to Layer stack
		 *
		 * The OnEvent call is basically made by the glfw library via m_Data.EventCallback. The registration is done like so
		 * @code{.cpp}
		 * m_Window->SetEventCallback(KR_BIND_EVENT_FN(Application::OnEvent)); // Setting the listener
		 * @endcode
		 * whilst, the EventCallback is called in the lambdas of (Linux/Mac/Windows)Window::SetGLFWCallbacks()
		 *
		 * @since Karma 1.0.0
		 */
		void OnEvent(Event& e);

		/**
		 * @brief Places the Layer in the stack of layers and sends a notification
		 *
		 * LayerStack::m_Layers.emplace() is used to place the supplied layer at an index in ascending order.
		 * For instance the layer inserted first has least index and subsequent layers have higher indices ordered accordingly.
		 *
		 * @param layer						The Layer to be placed
		 *
		 * @see LayerStack::PushLayer
		 * @since Karma 1.0.0
		 */
		void PushLayer(Layer* layer);

		/**
		 * @brief Places the Layer at the front most position, in the sense, after the last inserted layer.
		 *
		 * @param layer						The Layer to be pushed as overlay
		 *
		 * @see PushLayer
		 * @since Karma 1.0.0
		 */
		void PushOverlay(Layer* layer);

		/**
		 * @brief Do the preliminaries before the main loop run
		 *
		 * The following steps are taken
		 * 	- Input system is initialized
		 * 	- Memory system is initialized
		 * 	- Prepare an outer for KEngine instance
		 * 	- Initialize KEngine
		 *
		 * @since Karma 1.0.0
		 */
		void PrepareApplicationForRun();

		/**
		 * @brief Set an event callback to the given input system
		 *
		 * @todo Think about the need to uplift to more abstract implementation
		 * @since Karma 1.0.0
		 */
		void HookInputSystem(std::shared_ptr<Input> input);

		/**
		 * @brief All the bulk memory allocation is done to prevent frequent calls to
		 * context switching new/delete operators.
		 *
		 * @note See KarmaSmriti for understanding the workings of Karma's memory management
		 * @since Karma 1.0.0
		 */
		void PrepareMemorySoftBed();

		/**
		 * @brief Instantiate the instance of KEngine
		 *
		 * @todo Ponder over the usage of KEngine type instances for Editor and Game
		 * @since Karma 1.0.0
		 */
		void InitializeApplicationEngine();

		/**
		 * @brief Clean up all the KEngine relevant mess
		 *
		 * @todo Need to think how this should be written with GC in mind
		 * @since Karma 1.0.0
		 */
		void DecommisionApplicationEngine();

		/**
		 * @brief Getter for the instance (s_Instance) of the Application
		 *
		 * @since Karma 1.0.0
		 */
		inline static Application& Get() { return *s_Instance; }

		/**
		 * @brief Getter for the Window handle
		 *
		 * @since Karma 1.0.0
		 */
		inline Window& GetWindow() const { return *m_Window; }

		/**
		 * @brief Closing of the Application and do the relevant processing for clean up
		 *
		 * @since Karma 1.0.0
		 */
		void CloseApplication();

	private:
		/**
		 * @brief Privately defined Window close routine
		 *
		 * @param event						The window closing Event
		 * @since Karma 1.0.0
		 */
		bool OnWindowClose(WindowCloseEvent& event);

		/**
		 * @brief Privately defined Window resize routine
		 *
		 * @param event						The window resizing Event
		 * @since Karma 1.0.0
		 */
		bool OnWindowResize(WindowResizeEvent& event);

		/**
		 * @brief Privately defined routine for the Event when controller physical device is connected
		 *
		 * @param event						The device connection Event
		 * @since Karma 1.0.0
		 */
		bool OnControllerDeviceConnected(ControllerDeviceConnectedEvent& event);

		/**
		 * @brief Privately defined routine for the Event when the controller physical device is disconnected
		 *
		 * @param event						The device disconnection Event
		 * @since Karma 1.0.0
		 */
		bool OnControllerDeviceDisconnected(ControllerDeviceDisconnectedEvent& event);

		// We are using raw pointers because we want necessary control over the lifetime of
		// the objects. Especially for clearing up Vulkan relevant parts.
		Window* m_Window;
		LayerStack* m_LayerStack;

		KarmaGuiLayer* m_KarmaGuiLayer;
		bool m_Running = true;

		static Application* s_Instance;

		static RunningPlatform m_RPlatform;
	};

	extern KARMA_API KarmaSmriti m_MemoryManager;

	// To be defined in the client application
	/**
	 * @brief Declaration of client definable routine
	 *
	 * @note This provides a modularity with the client applications (in accordance with SR principle, the way I understand)
	 *
	 * @return An instance of main Application class
	 * @since Karma 1.0.0
	 */
	Application* CreateApplication();
}
