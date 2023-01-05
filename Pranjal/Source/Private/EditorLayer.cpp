#include "EditorLayer.h"

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
			modelMesh.reset(new Karma::Mesh("../Resources/Models/viking_room.obj"));

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
		m_ModelTexture.reset(new Karma::Texture(Karma::TextureType::Image, "../Resources/Textures/viking_room.png", "VikingTex", "texSampler"));

		m_ModelMaterial->AddTexture(m_ModelTexture);
		//m_ModelMaterial->AttatchMainCamera(m_EditorCamera); Is this needed?

		m_ModelVertexArray->SetMaterial(m_ModelMaterial);

		m_EditorScene.reset(new Karma::Scene());
		m_EditorScene->AddCamera(m_EditorCamera);
		m_EditorScene->AddVertexArray(m_ModelVertexArray);
		m_EditorScene->SetClearColor({ 0.0f, 0.0f, 0.0f, 1 });

		//Karma::Renderer::SetScene(m_EditorScene);
	}

	void EditorLayer::OnAttach()
	{

	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(float deltaTime)
	{

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
