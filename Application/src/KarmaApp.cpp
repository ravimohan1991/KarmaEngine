#include "Karma.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

class ExampleLayer : public Karma::Layer
{
public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		m_VertexArray.reset(Karma::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};

		std::shared_ptr<Karma::VertexBuffer> m_VertexBuffer;
		m_VertexBuffer.reset(Karma::VertexBuffer::Create(vertices, sizeof(vertices)));

		{
			Karma::BufferLayout layout = {
				{ Karma::ShaderDataType::Float3, "a_Position" },
				{ Karma::ShaderDataType::Float4, "a_Color"	}
			};

			m_VertexBuffer->SetLayout(layout);
		}

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };

		std::shared_ptr<Karma::IndexBuffer> m_IndexBuffer;
		m_IndexBuffer.reset(Karma::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		std::shared_ptr<Karma::UniformBufferObject> shaderUniform;
		shaderUniform.reset(Karma::UniformBufferObject::Create({ Karma::ShaderDataType::Mat4, Karma::ShaderDataType::Mat4 }, 0));
		m_Shader.reset(Karma::Shader::Create("../Resources/Shaders/shader.vert", "../Resources/Shaders/shader.frag", shaderUniform, true));
		m_VertexArray->SetShader(m_Shader);

		// Drawing square
		m_SquareVA.reset(Karma::VertexArray::Create());

		float verticesBSQ[7 * 4] = {
			-0.25f, -0.25f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.25f, -0.25f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.25f, 0.25f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 -0.25f, 0.25f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f
		};

		std::shared_ptr<Karma::VertexBuffer> squareVB;
		squareVB.reset(Karma::VertexBuffer::Create(verticesBSQ, sizeof(verticesBSQ)));

		{
			Karma::BufferLayout layout = {
				{ Karma::ShaderDataType::Float3, "a_Position" },
				{ Karma::ShaderDataType::Float4, "lol" }
			};

			squareVB->SetLayout(layout);
		}

		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t indicesBSQ[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Karma::IndexBuffer> squareIB;
		squareIB.reset(Karma::IndexBuffer::Create(indicesBSQ, sizeof(indicesBSQ) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		m_BlueSQShader.reset(Karma::Shader::Create("../Resources/Shaders/shader.vert", "../Resources/Shaders/shader.frag", shaderUniform, true));
		m_SquareVA->SetShader(m_BlueSQShader);
	}

	virtual void OnUpdate(float deltaTime) override
	{
		KarmaAppInputPolling(deltaTime);

		Karma::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Karma::RenderCommand::Clear();

		m_Camera.SetPosition({ camData.x_Pos, camData.y_Pos, 0.0f });
		m_Camera.SetRotation(camData.angle);
		
		Karma::Renderer::BeginScene(m_Camera);
		
		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
		Karma::UBODataPointer uViewProjection(&m_Camera.GetViewProjectionMatrix());
		glm::mat4 transform = glm::mat4(1.0f);
		Karma::UBODataPointer uTransform(&transform);

		m_Shader->GetUniformBufferObject()->UpdateUniforms(uViewProjection, uTransform);
		Karma::Renderer::Submit(m_VertexArray, m_Shader);
		/*
		for (int h = 0; h < 20; h++)
		{
			for (int i = 0; i < 20; i++)
			{
				glm::vec3 pos(i * 0.11f, h * 0.11f, 0.0f);
				transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				m_BlueSQShader->GetUniformBufferObject()->UpdateUniforms(uViewProjection, uTransform);
				
				Karma::Renderer::Submit(m_SquareVA, m_BlueSQShader);
			}
		}*/

		Karma::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
	}

	virtual void OnEvent(Karma::Event& e) override
	{
	}

	void KarmaAppInputPolling(float deltaTime)
	{
		// Camera controls
		if (Karma::Input::IsKeyPressed(GLFW_KEY_A))
		{
			camData.x_Pos -= cameraTranslationSpeed * deltaTime;
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_D))
		{
			camData.x_Pos += cameraTranslationSpeed * deltaTime;
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_W))
		{
			camData.y_Pos += cameraTranslationSpeed * deltaTime;
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_S))
		{
			camData.y_Pos -= cameraTranslationSpeed * deltaTime;
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_R))
		{
			camData.angle += cameraRotationSpeed * deltaTime;
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_T))
		{
			camData.angle -= cameraRotationSpeed * deltaTime;
		}
	}

private:
	std::shared_ptr<Karma::Shader> m_Shader;
	std::shared_ptr<Karma::Shader> m_BlueSQShader;

	std::shared_ptr<Karma::VertexArray> m_VertexArray;
	std::shared_ptr<Karma::VertexArray> m_SquareVA;

	Karma::OrthographicCamera m_Camera;
	struct CameraData
	{
		float x_Pos = 0.0f;
		float y_Pos = 0.0f;
		float angle = 0.0f;
	};
	CameraData camData;
	float cameraTranslationSpeed = 1.0f;
	float cameraRotationSpeed = 180.0f;
};

class VulkanLayer : public Karma::Layer
{
public:
	VulkanLayer() : m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		m_VertexArray.reset(Karma::VertexArray::Create());
		/*float vertices[3 * 7] = {
			0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};*/

		float vertices[7 * 5] = {
			0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
		};
		
		std::shared_ptr<Karma::VertexBuffer> m_VertexBuffer;
		m_VertexBuffer.reset(Karma::VertexBuffer::Create(vertices, sizeof(vertices)));

		{
			Karma::BufferLayout layout = {
				{Karma::ShaderDataType::Float3, "a_Positon"},
				{Karma::ShaderDataType::Float4, "a_Color"} };
		
			m_VertexBuffer->SetLayout(layout);
		}
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indices[9] = { 0, 1, 2, 2, 3, 0, 2, 4, 3 };

		std::shared_ptr<Karma::IndexBuffer> m_IndexBuffer;
		m_IndexBuffer.reset(Karma::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		std::shared_ptr<Karma::UniformBufferObject> shaderUniform;
		shaderUniform.reset(Karma::UniformBufferObject::Create({ Karma::ShaderDataType::Mat4, Karma::ShaderDataType::Mat4 }, 0));
		m_Shader.reset(Karma::Shader::Create("../Resources/Shaders/shader.vert", "../Resources/Shaders/shader.frag", shaderUniform, true));
		m_VertexArray->SetShader(m_Shader);
	}

	virtual void OnUpdate(float deltaTime) override
	{
		//KR_INFO("DeltaTime = {0} ms", deltaTime * 1000.0f);
		
		Karma::Renderer::BeginScene(m_Camera);

		Karma::Renderer::Submit(m_VertexArray, m_Shader);

		Karma::Renderer::EndScene();
	}

private:
	std::shared_ptr<Karma::VertexArray> m_VertexArray;
	Karma::OrthographicCamera m_Camera;
	std::shared_ptr<Karma::Shader> m_Shader;
};

class KarmaApp : public Karma::Application
{
public:
	KarmaApp()
	{
		PushLayer(new ExampleLayer());
		//PushLayer(new VulkanLayer());
	}

};

Karma::Application* Karma::CreateApplication()
{ 
	return new KarmaApp();
}