#include "Application.h"
#include "Karma/Log.h"
#include "Karma/Input.h"
#include "Karma/Renderer/Renderer.h"
#include "chrono"
#include "Engine/Engine.h"
#include "Core/UObjectGlobals.h"// to be bundled appropriately in core.h
#include "Core/Package.h"

namespace Karma
{
	Application* Application::s_Instance = nullptr;

#ifdef KR_WINDOWS_PLATFORM
	Karma::RunningPlatform Karma::Application::m_RPlatform = Karma::RunningPlatform::Windows;
#elif KR_LINUX_PLATFORM
	Karma::RunningPlatform Karma::Application::m_RPlatform = Karma::RunningPlatform::Linux;
#elif KR_MAC_PLATFORM
	Karma::RunningPlatform Karma::Application::m_RPlatform = Karma::RunningPlatform::Mac;
#endif

	// Think if singelton pattern is to be used or something else
	// http://gameprogrammingpatterns.com/singleton.html
	Karma::KarmaSmriti m_MemoryManager;

	Application::Application()
	{
		KR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create();
		m_Window->SetEventCallback(KR_BIND_EVENT_FN(Application::OnEvent)); // Setting the listener

		m_LayerStack = new LayerStack();

		// Graphics API Vulkan or OpenGL should have been completely initialized by here
		m_KarmaGuiLayer = new KarmaGuiLayer(m_Window);
		PushOverlay(m_KarmaGuiLayer);
	}

	Application::~Application()
	{
		// Deinitialize Kengine

		m_MemoryManager.ShutDown();
		Renderer::DeleteData();
		// We want to clear off layers and their rendering components before the m_Window
		// and its context.
		KR_CORE_INFO("Deleting stacks");
		delete m_LayerStack;
		KR_CORE_INFO("Deleting window");
		delete m_Window;
		s_Instance = nullptr;
	}

	void Application::PrepareApplicationForRun()
	{
		HookInputSystem(Input::GetInputInstance());
		PrepareMemorySoftBed();

		StaticUObjectInit();
		// Initialize KEngine
		InitializeApplicationEngine();
	}

	void Application::InitializeApplicationEngine()
	{
		GEngine = NewObject<KEngine>(GetTransientPackage(), KEngine::StaticClass(), "KEngine");
	}

	void Application::DecommisionApplicationEngine()
	{

	}

	// May need to uplift to more abstract implementation
	void Application::HookInputSystem(std::shared_ptr<Input> input)
	{
		input->SetEventCallback(KR_BIND_EVENT_FN(Application::OnEvent), m_Window);
	}

	void Application::PrepareMemorySoftBed()
	{
		m_MemoryManager.StartUp();
		KR_CORE_INFO("Prepared Karma's soft memory bed for resource allocation");
	}

	void Application::Run()
	{
		std::chrono::high_resolution_clock::time_point begin, end;

		begin = std::chrono::high_resolution_clock::now();

		while (m_Running)
		{
			end = std::chrono::high_resolution_clock::now();

			float deltaTime = (float)std::chrono::duration_cast<std::chrono::microseconds>
				(end - begin).count();
			begin = end;

			deltaTime /= 1000000.0f;

			// Tick KEngine
			GEngine->Tick(deltaTime, false);

			for (auto layer : *m_LayerStack)
			{
				layer->OnUpdate(deltaTime);
			}

			// ImGui rendering sequence cue trickling through stack
			m_KarmaGuiLayer->Begin();

			for (auto layer : *m_LayerStack)
			{
				layer->ImGuiRender(deltaTime);
			}

			m_KarmaGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		m_Running = false;

		return true;
	}

	void Application::CloseApplication()
	{
		m_Running = false;

		// Do leftover work like memorizing window size and all that
	}

	bool Application::OnWindowResize(WindowResizeEvent& event)
	{
		return m_Window->OnResize(event);
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack->PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack->PushOverlay(layer);
		layer->OnAttach();
	}

	bool Application::OnControllerDeviceConnected(ControllerDeviceConnectedEvent& event)
	{
		KR_CORE_INFO("Application receieved Controller ConnectionEvent");
		return true;
	}

	bool Application::OnControllerDeviceDisconnected(ControllerDeviceDisconnectedEvent& event)
	{
		KR_CORE_INFO("Application receieved Controller DisconnectionEvent");
		return true;
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(KR_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(KR_BIND_EVENT_FN(Application::OnWindowResize));
		dispatcher.Dispatch<ControllerDeviceConnectedEvent>(KR_BIND_EVENT_FN(Application::OnControllerDeviceConnected));
		dispatcher.Dispatch<ControllerDeviceDisconnectedEvent>(KR_BIND_EVENT_FN(Application::OnControllerDeviceDisconnected));

		for (auto it = m_LayerStack->end(); it != m_LayerStack->begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.IsHandled())
			{
				break;
			}
		}
	}
}
