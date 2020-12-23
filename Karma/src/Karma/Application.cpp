#include "Application.h"
#include "Karma/Log.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Karma/Input.h"

namespace Karma
{
	Application* Application::s_Instance = nullptr;

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Karma::ShaderDataType::Float:
				return GL_FLOAT;
			case Karma::ShaderDataType::Float2:
				return GL_FLOAT;
			case Karma::ShaderDataType::Float3:
				return GL_FLOAT;
			case Karma::ShaderDataType::Float4:
				return GL_FLOAT;
			case Karma::ShaderDataType::Mat3:
				return GL_FLOAT;
			case Karma::ShaderDataType::Mat4:
				return GL_FLOAT;
			case Karma::ShaderDataType::Int:
				return GL_INT;
			case Karma::ShaderDataType::Int2:
				return GL_INT;
			case Karma::ShaderDataType::Int3:
				return GL_INT;
			case Karma::ShaderDataType::Int4:
				return GL_INT;
			case Karma::ShaderDataType::Bool:
				return GL_INT;
		}

		KR_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	Application::Application()
	{
		KR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(KR_BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		// Vertex buffer
		/*glGenBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);*/

		// Clip space coordinates
		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};

		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"	}
		};
		
		uint32_t index = 0;
		
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, element.GetComponentCount(), 
				ShaderDataTypeToOpenGLBaseType(element.Type), 
				element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), 
				(const void*)element.Offset);
			index++;
		}

		// Upload to GPU
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Index buffer
		/*glGenBuffers(1, &m_IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);*/

		uint32_t indices[3] = { 0, 1, 2 };

		m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		
		// Upload to GPU
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec4 v_Color;
			
			void main()
			{
				gl_Position = vec4(a_Position, 1.0f);
				v_Color = a_Color;
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec4 v_Color;

			void main()
			{
				color = vec4(0.8, 0.2, 0.3, 1.0);
				color = v_Color;
			}
		)";

		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
	}

	Application::~Application()
	{
		Input::DeleteInstance();
		s_Instance = nullptr;
	}
	
	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			
			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

			// The range based for loop valid because we have implemented begin()
			// and end() in LayerStack.h
			for (auto layer : m_LayerStack)
			{
				layer->OnUpdate();
			}
			
			m_ImGuiLayer->Begin();
			for (auto layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

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
