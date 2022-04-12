#include "Karma.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

class ExampleLayer : public Karma::Layer
{
public:
	ExampleLayer() : Layer("Example"), m_Camera(45.0f, 1280.f / 720.0f, 0.1f, 10.0f) /*m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)*/
	{
		// Drawing boned cylinder
		m_SquareVA.reset(Karma::VertexArray::Create());
		Karma::ModelLoader* twoBonedCylinder = new Karma::ModelLoader("../Resources/Models/BonedCylinder.obj");

		std::shared_ptr<Karma::VertexBuffer> squareVB;
		squareVB.reset(Karma::VertexBuffer::Create(twoBonedCylinder->GetVertexData(), twoBonedCylinder->GetVertexSize()));

		{
			Karma::BufferLayout layout = {
				{ Karma::ShaderDataType::Float3, "a_Position" },
				{ Karma::ShaderDataType::Float4, "a_Color" },
				{ Karma::ShaderDataType::Float2, "a_UVs" }
			};

			squareVB->SetLayout(layout);
		}

		m_SquareVA->AddVertexBuffer(squareVB);


		std::shared_ptr<Karma::IndexBuffer> squareIB;
		squareIB.reset(Karma::IndexBuffer::Create(twoBonedCylinder->GetIndexData(), twoBonedCylinder->GetIndexCount()));

		m_SquareVA->SetIndexBuffer(squareIB);

		std::shared_ptr<Karma::UniformBufferObject> shaderUniform;
		shaderUniform.reset(Karma::UniformBufferObject::Create({ Karma::ShaderDataType::Mat4, Karma::ShaderDataType::Mat4 }, 0));

		m_BlueSQShader.reset(Karma::Shader::Create("../Resources/Shaders/shader.vert", "../Resources/Shaders/shader.frag", shaderUniform, true));
		m_SquareVA->SetShader(m_BlueSQShader);

		delete twoBonedCylinder;
	}

	virtual void OnUpdate(float deltaTime) override
	{
		KarmaAppInputPolling(deltaTime);

		Karma::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });
		Karma::RenderCommand::Clear();

		Karma::Renderer::BeginScene(m_Camera);
		
		Karma::UBODataPointer uProjection(&m_Camera.GetProjectionMatrix());
		Karma::UBODataPointer uView(&m_Camera.GetViewMatirx());

		//KR_INFO("DeltaTime = {0} ms", deltaTime * 1000.0f);

		m_BlueSQShader->GetUniformBufferObject()->UpdateUniforms(uProjection, uView);

		Karma::Renderer::Submit(m_SquareVA, m_BlueSQShader);
		
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
			m_Camera.MoveSideways(-cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_D))
		{
			m_Camera.MoveSideways(cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_W))
		{
			m_Camera.MoveForward(cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_S))
		{
			m_Camera.MoveForward(-cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1))
		{
			m_Camera.LeftMouseButtonPressed();
		}

		if (Karma::Input::IsMouseButtonReleased(GLFW_MOUSE_BUTTON_1))
		{
			m_Camera.LeftMouseButtonReleased();
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_SPACE))
		{
			m_Camera.MoveUp(cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		{
			m_Camera.MoveUp(-cameraTranslationSpeed * deltaTime);
		}
	}

private:
	std::shared_ptr<Karma::Shader> m_BlueSQShader;

	std::shared_ptr<Karma::VertexArray> m_SquareVA;

	Karma::PerspectiveCamera m_Camera;

	float cameraTranslationSpeed = 1.0f;
	float cameraRotationSpeed = 180.0f;
};

class KarmaApp : public Karma::Application
{
public:
	KarmaApp()
	{
		PushLayer(new ExampleLayer());
	}

};

Karma::Application* Karma::CreateApplication()
{ 
	return new KarmaApp();
}