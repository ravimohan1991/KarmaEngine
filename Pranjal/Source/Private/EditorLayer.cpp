#include "EditorLayer.h"
#include "Karma/ImGui/ImGuiMesa.h"

#include "GLFW/glfw3.h"

namespace Karma
{
	EditorLayer::EditorLayer() : Layer("Pranjal")
	{
		// Instantiate camera
		m_EditorCamera.reset(new Karma::PerspectiveCamera(45.0f, 1280.f / 720.0f, 0.1f, 100.0f));

		// Model loading begins

		// First intantiate VertexArray
		m_ModelVertexArray.reset(Karma::VertexArray::Create());

		{
			// Get hold of model
			std::shared_ptr<Karma::Mesh> modelMesh;
			modelMesh.reset(new Karma::Mesh("../Resources/Models/BonedCylinder.obj"));

			// Set the mesh in vertex array
			m_ModelVertexArray->SetMesh(modelMesh);
		}

		// Next, instantiate material
		m_ModelMaterial.reset(new Karma::Material());

		{
			// Setting shader

			// Uniforms for regular transform uploads
			std::shared_ptr<Karma::UniformBufferObject> shaderUniform;
			shaderUniform.reset(Karma::UniformBufferObject::Create({ Karma::ShaderDataType::Mat4, Karma::ShaderDataType::Mat4 }, 0));
			
			m_ModelShader.reset(Karma::Shader::Create("../Resources/Shaders/shader.vert", "../Resources/Shaders/shader.frag", shaderUniform, true, "CylinderShader"));

			m_ModelMaterial->AddShader(m_ModelShader);
		}

		// Then we set texture
		m_ModelTexture.reset(new Karma::Texture(Karma::TextureType::Image, "../Resources/Textures/UnrealGrid.png", "VikingTex", "texSampler"));

		m_ModelMaterial->AddTexture(m_ModelTexture);
		m_ModelMaterial->AttatchMainCamera(m_EditorCamera); //Is this needed?

		m_ModelVertexArray->SetMaterial(m_ModelMaterial);

		m_EditorScene.reset(new Karma::Scene());
		m_EditorScene->AddCamera(m_EditorCamera);
		m_EditorScene->AddVertexArray(m_ModelVertexArray);
		m_EditorScene->SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });
	}

	void EditorLayer::OnAttach()
	{

	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(float deltaTime)
	{
		InputPolling(deltaTime);
	}

	void EditorLayer::ImGuiRender(float deltaTime)
	{
		ImGuiID dockspaceID;
		// 1. Show the big demo window. For debug purpose!!
		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		// 2. A UI canvas, if I may, for the main window!!
		{
			static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
			ImGuiWindowFlags windowFlags = ImGuiDockNodeFlags_None;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);

			// No clue
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			// hmm
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::Begin("KarmaSafetyDockSpace", nullptr, windowFlags);
			ImGui::PopStyleVar();
			ImGui::PopStyleVar(2);

			dockspaceID = ImGui::GetID("KarmaSafetyDockSpace");
			ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

			ImGui::End();
		}

		// The complete UI Karma shall (ever?) need. Not counting meta morpho analytic and service toolset
		{
			ImGuiMesa::RevealMainFrame(dockspaceID, m_EditorScene);
		}
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == GLFW_MOUSE_BUTTON_1)
		{
			if (ImGuiMesa::m_ViewportHovered)
			{
			}
		}
		return false;
	}

	bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
	{
		m_EditorCamera->LeftMouseButtonReleased();
		return false;
	}

	void EditorLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<KeyPressedEvent>(KR_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(KR_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));

		dispatcher.Dispatch<MouseButtonReleasedEvent>(KR_BIND_EVENT_FN(EditorLayer::OnMouseButtonReleased));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		return false;
	}

	void EditorLayer::InputPolling(float deltaTime)
	{
		// Camera controls
		if (Input::IsKeyPressed(GLFW_KEY_A))
		{
			m_EditorCamera->MoveSideways(-cameraTranslationSpeed * deltaTime);
		}

		if (Input::IsKeyPressed(GLFW_KEY_D))
		{
			m_EditorCamera->MoveSideways(cameraTranslationSpeed * deltaTime);
		}

		if (Input::IsKeyPressed(GLFW_KEY_W))
		{
			m_EditorCamera->MoveForward(cameraTranslationSpeed * deltaTime);
		}

		if (Input::IsKeyPressed(GLFW_KEY_S))
		{
			m_EditorCamera->MoveForward(-cameraTranslationSpeed * deltaTime);
		}

		static uint32_t testControllerID = 0;

		// Controller context begins
		float val = Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_LEFT_Y, testControllerID);

		if (abs(val) >= .1f)
		{
			m_EditorCamera->MoveForward(-1.f * val * cameraTranslationSpeed * deltaTime);
		}

		val = Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_LEFT_X, testControllerID);

		if (abs(val) >= .1f)
		{
			m_EditorCamera->MoveSideways(val * cameraTranslationSpeed * deltaTime);
		}
		// Controller context ends


		if (ImGuiMesa::m_ViewportHovered)
		{
			if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1))
			{
				m_EditorCamera->LeftMouseButtonPressed();
			}

			if (Input::IsMouseButtonReleased(GLFW_MOUSE_BUTTON_1))
			{
				m_EditorCamera->LeftMouseButtonReleased();
			}
		}

		if (Input::IsKeyPressed(GLFW_KEY_SPACE))
		{
			m_EditorCamera->MoveUp(cameraTranslationSpeed * deltaTime);
		}

		if (Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		{
			m_EditorCamera->MoveUp(-cameraTranslationSpeed * deltaTime);
		}

		// Controller context begins
		if (Input::IsControllerButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, testControllerID))
		{
			m_EditorCamera->MoveUp(-cameraTranslationSpeed * deltaTime);
		}

		if (Input::IsControllerButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP, testControllerID))
		{
			m_EditorCamera->MoveUp(cameraTranslationSpeed * deltaTime);
		}

		val = Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_RIGHT_X, testControllerID);

		if (abs(val) > .1f)
		{
			m_EditorCamera->RotateAboutYAxis(val * cameraRotationSpeed * deltaTime);
		}

		val = Input::ControllerAxisPivotVal(GLFW_GAMEPAD_AXIS_RIGHT_Y, testControllerID);
		if (abs(val) > .1f)
		{
			m_EditorCamera->RotateAboutXAxis(-1.f * val * cameraRotationSpeed * deltaTime);
		}
		// Controller context ends
	}

}

class KarmaApp : public Karma::Application
{
public:
	KarmaApp()
	{
		PushLayer(new Karma::EditorLayer());
	}
};

Karma::Application* Karma::CreateApplication()
{
	return new KarmaApp();
}
