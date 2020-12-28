#pragma once

#include "Karma/Core.h"
#include "Karma/Window.h"
#include "Karma/Events/ApplicationEvent.h"
#include "Karma/LayerStack.h"
#include "Karma/ImGui/ImGuiLayer.h"
#include "Karma/Renderer/Shader.h"
#include "Karma/Renderer/Buffer.h"
#include "Karma/Renderer/VertexArray.h"
#include "Karma/Renderer/OrthographicCamera.h"

#include <memory>

namespace Karma
{
	class KARMA_API Application
	{
	public:
		Application();
		~Application();
		
		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() const { return *m_Window; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;// Created on stack. For entire lifetime of the program (singleton?)

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<Shader> m_BlueSQShader;
	
		std::shared_ptr<VertexArray> m_VertexArray;
		std::shared_ptr<VertexArray> m_SquareVA;

		OrthographicCamera m_Camera;

		static Application* s_Instance;
	};

	// To be defined in the client application
	Application* CreateApplication();
}
