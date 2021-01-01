#define GLFW_INCLUDE_VULKAN
#include "Application.h"
#include "Karma/Log.h"
#include "GLFW/glfw3.h"
#include "Karma/Input.h"
#include "Karma/Renderer/Renderer.h"
#include "chrono"

namespace Karma
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		KR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(KR_BIND_EVENT_FN(Application::OnEvent));
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		KR_CORE_INFO("{0} Vulkan extensions supported", extensionCount);

		//m_ImGuiLayer = new ImGuiLayer();
		//PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		Input::DeleteInstance();
		s_Instance = nullptr;
	}
	
	void Application::Run()
	{
		std::chrono::high_resolution_clock::time_point begin, end;

		begin = std::chrono::high_resolution_clock::now();

		while (m_Running)
		{
			end = std::chrono::high_resolution_clock::now();

			float deltaTime = (float) std::chrono::duration_cast<std::chrono::microseconds>
				(end - begin).count();
			begin = end;
			
			deltaTime /= 1000000.0f;

			// The range based for loop valid because we have implemented begin()
			// and end() in LayerStack.h
			for (auto layer : m_LayerStack)
			{
				layer->OnUpdate(deltaTime);
			}
			
			/*m_ImGuiLayer->Begin();
			for (auto layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();*/

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		m_Running = false;

		return true;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}
	
	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(KR_BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.IsHandled())
			{
				break;
			}
		}
	}
}
