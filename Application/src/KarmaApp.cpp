#include "Karma.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Karma/Renderer/Material.h"
#include "Karma/Renderer/Texture.h"

class ExampleLayer : public Karma::Layer
{
public:
	ExampleLayer() : Layer("Example") /*m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)*/
	{
		m_Camera.reset(new Karma::PerspectiveCamera(45.0f, 1280.f / 720.0f, 0.1f, 100.0f));
		
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

		m_BlueSQShader.reset(Karma::Shader::Create("../Resources/Shaders/shader.vert", "../Resources/Shaders/shader.frag", shaderUniform, true, "CylinderShader"));
		
		m_SquareMat.reset(new Karma::Material());
		m_SquareMat->AddShader(m_BlueSQShader);
		m_SquareTex.reset(new Karma::Texture(Karma::TextureType::Image, "../Resources/Textures/viking_room.png", "VikingTex", "texSampler"));
		m_SquareMat->AddTexture(m_SquareTex);
		
		m_SquareMat->AttatchMainCamera(m_Camera);

		// Should be Material
		m_SquareVA->SetShader(m_BlueSQShader);

		delete twoBonedCylinder;
	}

	virtual void OnUpdate(float deltaTime) override
	{
		KarmaAppInputPolling(deltaTime);

		Karma::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });
		Karma::RenderCommand::Clear();

		Karma::Renderer::BeginScene(*m_Camera);

		//KR_INFO("DeltaTime = {0} ms", deltaTime * 1000.0f);
		
		// May need entry point for Object's world transform
		m_SquareMat->OnUpdate();
		
		// Cluster in Vertex Array Process perhabs
		m_SquareMat->ProcessForSubmission();
		m_SquareVA->Bind();

		Karma::Renderer::Submit(m_SquareVA);
		
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
			m_Camera->MoveSideways(-cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_D))
		{
			m_Camera->MoveSideways(cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_W))
		{
			m_Camera->MoveForward(cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_S))
		{
			m_Camera->MoveForward(-cameraTranslationSpeed * deltaTime);
		}


		// Controller context begins
		float mag = Karma::Input::ControllerAxisPivotMag(GLFW_GAMEPAD_AXIS_LEFT_Y, 0);

		if (abs(mag) >= .1f)
		{
			m_Camera->MoveForward(-1.f * mag * cameraTranslationSpeed * deltaTime);
		}

		mag = Karma::Input::ControllerAxisPivotMag(GLFW_GAMEPAD_AXIS_LEFT_X, 0);

		if (abs(mag) >= .1f)
		{
			m_Camera->MoveSideways(mag * cameraTranslationSpeed * deltaTime);
		}
		// Controller context ends


		if (Karma::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1))
		{
			m_Camera->LeftMouseButtonPressed();
		}

		if (Karma::Input::IsMouseButtonReleased(GLFW_MOUSE_BUTTON_1))
		{
			m_Camera->LeftMouseButtonReleased();
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_SPACE))
		{
			m_Camera->MoveUp(cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		{
			m_Camera->MoveUp(-cameraTranslationSpeed * deltaTime);
		}

		// Controller context begins
		if (Karma::Input::IsControllerButtonPressed(GLFW_GAMEPAD_BUTTON_A, 0))
		{
			m_Camera->MoveUp(-cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsControllerButtonPressed(GLFW_GAMEPAD_BUTTON_B, 0))
		{
			m_Camera->MoveUp(cameraTranslationSpeed * deltaTime);
		}

		mag = Karma::Input::ControllerAxisPivotMag(GLFW_GAMEPAD_AXIS_RIGHT_X, 0);

		if (abs(mag) > .1f)
		{
			m_Camera->RotateAboutYAxis(mag * cameraRotationSpeed * deltaTime);
		}

		mag = Karma::Input::ControllerAxisPivotMag(GLFW_GAMEPAD_AXIS_RIGHT_Y, 0);
		if (abs(mag) > .1f)
		{
			m_Camera->RotateAboutXAxis(-1.f * mag * cameraRotationSpeed * deltaTime);
		}
		// Controller context ends
	}

private:
	std::shared_ptr<Karma::Shader> m_BlueSQShader;

	std::shared_ptr<Karma::VertexArray> m_SquareVA;
	std::shared_ptr<Karma::Material> m_SquareMat;
	std::shared_ptr<Karma::Texture> m_SquareTex;

	std::shared_ptr<Karma::PerspectiveCamera> m_Camera;

	float cameraTranslationSpeed = 1.0f;
	float cameraRotationSpeed = 80.0f;
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