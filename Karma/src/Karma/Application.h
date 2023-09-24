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
	enum class RunningPlatform
	{
		Linux = 0,
		Mac,
		Windows
	};

	class KARMA_API Application
	{
	public:
		Application();
		~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void PrepareApplicationForRun();

		void HookInputSystem(std::shared_ptr<Input> input);

		/**
		 * All the bulk memory allocation is done to prevent frequent calls to
		 * context swithing new/delete operators. 
		 */
		void PrepareMemorySoftBed();

		void InitializeApplicationEngine();
		void DecommisionApplicationEngine();

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() const { return *m_Window; }

		void CloseApplication();

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);
		bool OnControllerDeviceConnected(ControllerDeviceConnectedEvent& event);
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
	Application* CreateApplication();
}
