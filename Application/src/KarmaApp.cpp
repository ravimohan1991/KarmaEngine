#include "Karma.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

class ExampleLayer : public Karma::Layer
{
public:
	ExampleLayer() : Layer("Example") /*m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)*/
	{
		m_Camera.reset(new Karma::PerspectiveCamera(45.0f, 1280.f / 720.0f, 0.1f, 100.0f));
		
		Karma::SceneModel* sModel = new Karma::SceneModel("../Resources/Models/BonedCylinder.obj");
		delete sModel;
	}

	virtual void OnUpdate(float deltaTime) override
	{
		KarmaAppInputPolling(deltaTime);

		Karma::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });
		Karma::RenderCommand::Clear();

		Karma::Renderer::BeginScene(*m_Camera);

		//KR_INFO("DeltaTime = {0} ms", deltaTime * 1000.0f);
		
		// May need entry point for Object's world transform
		//m_SquareMat->OnUpdate();
		
		// Cluster in Vertex Array Process perhabs
		//m_SquareMat->ProcessForSubmission();
		//m_SquareVA->Bind();

		//Karma::Renderer::Submit(m_SquareVA);
		
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
		float val = Karma::Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_LEFT_Y, 0);

		if (abs(val) >= .1f)
		{
			m_Camera->MoveForward(-1.f * val * cameraTranslationSpeed * deltaTime);
		}

		val = Karma::Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_LEFT_X, 0);

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
		if (Karma::Input::IsControllerButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, 0))
		{
			m_Camera->MoveUp(-cameraTranslationSpeed * deltaTime);
		}

		if (Karma::Input::IsControllerButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP, 0))
		{
			m_Camera->MoveUp(cameraTranslationSpeed * deltaTime);
		}

		val = Karma::Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_RIGHT_X, 0);

		if (abs(val) > .1f)
		{
			m_Camera->RotateAboutYAxis(val * cameraRotationSpeed * deltaTime);
		}

		val = Karma::Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_RIGHT_Y, 0);
		if (abs(val) > .1f)
		{
			m_Camera->RotateAboutXAxis(-1.f * val * cameraRotationSpeed * deltaTime);
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