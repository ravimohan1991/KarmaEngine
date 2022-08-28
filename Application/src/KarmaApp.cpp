#include "Karma.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

class ExampleLayer : public Karma::Layer
{
public:
	ExampleLayer() : Layer("Example") /*m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)*/
	{
		m_Camera.reset(new Karma::PerspectiveCamera(45.0f, 1280.f / 720.0f, 0.1f, 100.0f));

		m_SquareVA.reset(Karma::VertexArray::Create());

		/*
		Karma::SceneModel* sModel = new Karma::SceneModel("../Resources/Models/viking_room.obj");
		m_SquareVA->SetMesh(sModel->GetMeshList()[0]);
		delete sModel;*/


		std::shared_ptr<Karma::Mesh> trialMesh;
		trialMesh.reset(new Karma::Mesh("../Resources/Models/BonedCylinder.obj"));
		m_SquareVA->SetMesh(trialMesh);

		std::shared_ptr<Karma::UniformBufferObject> shaderUniform;
		shaderUniform.reset(Karma::UniformBufferObject::Create({ Karma::ShaderDataType::Mat4, Karma::ShaderDataType::Mat4 }, 0));

		m_BlueSQShader.reset(Karma::Shader::Create("../Resources/Shaders/shader.vert", "../Resources/Shaders/shader.frag", shaderUniform, true, "CylinderShader"));

		m_SquareMat.reset(new Karma::Material());
		m_SquareMat->AddShader(m_BlueSQShader);
		m_SquareTex.reset(new Karma::Texture(Karma::TextureType::Image, "../Resources/Textures/viking_room.png", "VikingTex", "texSampler"));
		m_SquareMat->AddTexture(m_SquareTex);

		m_SquareMat->AttatchMainCamera(m_Camera);

		m_SquareVA->SetMaterial(m_SquareMat);

		// Should be Material
		//m_SquareVA->SetShader(m_BlueSQShader);
	}

	virtual void OnUpdate(float deltaTime) override
	{
		KarmaAppInputPolling(deltaTime);

		Karma::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });

		Karma::RenderCommand::Clear();

		Karma::Renderer::BeginScene(*m_Camera);

		//KR_INFO("DeltaTime = {0} ms", deltaTime * 1000.0f);
		m_SquareVA->UpdateProcessAndSetReadyForSubmission();
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

		static uint32_t testControllerID = 0;

		// Controller context begins
		float val = Karma::Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_LEFT_Y, testControllerID);

		if (abs(val) >= .1f)
		{
			m_Camera->MoveForward(-1.f * val * cameraTranslationSpeed * deltaTime);
		}

		val = Karma::Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_LEFT_X, testControllerID);

		if (abs(val) >= .1f)
		{
			m_Camera->MoveSideways(val * cameraTranslationSpeed * deltaTime);
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
		if (Karma::Input::IsControllerButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, testControllerID))
		{
			m_Camera->MoveUp(-cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsControllerButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP, testControllerID))
		{
			m_Camera->MoveUp(cameraTranslationSpeed * deltaTime);
		}

		val = Karma::Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_RIGHT_X, testControllerID);

		if (abs(val) > .1f)
		{
			m_Camera->RotateAboutYAxis(val * cameraRotationSpeed * deltaTime);
		}

		val = Karma::Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_RIGHT_Y, testControllerID);
		if (abs(val) > .1f)
		{
			m_Camera->RotateAboutXAxis(-1.f * val * cameraRotationSpeed * deltaTime);
		}
		// Controller context ends
	}

	virtual void OnAttach() override
	{
	}

	virtual void OnDetach() override
	{
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
