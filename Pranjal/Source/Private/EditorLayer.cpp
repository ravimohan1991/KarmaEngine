#include "EditorLayer.h"
#include "KarmaGuiMesa.h"
// Experimental
#include "GLFW/glfw3.h"
#include "Core/Object.h"
#include "Core/Class.h"
#include "Core/Package.h"
#include "ChildActorComponent.h"
#include "ActorIterator.h"
#include "UObjectIterator.h"
#include "Engine.h"
#include "GameInstance.h"

namespace Karma
{
	EditorLayer::EditorLayer() : Layer("Pranjal")
	{
		testWorld = nullptr;
		m_ActorCounter = 0;

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

	EditorLayer::~EditorLayer()
	{
		KarmaGuiMesa::MesaShutDownRoutine();
	}

	void EditorLayer::OpenScene(const std::string& objFileName)
	{
		std::shared_ptr<Mesh> meshToLoad;

		// Check if the already assigned is freed and there is no leak
		meshToLoad.reset(new Mesh(objFileName));

		m_ModelVertexArray->SetMesh(meshToLoad);
		KR_INFO("Successfully loaded scene");
	}

	void EditorLayer::OnAttach()
	{
		// Load Images
		//	1. The wall
		KarmaGuiRenderer::AddImageTexture("../Resources/Textures/The_Source_Wall.jpg", "The Source");

		// 2. 3D Exhibitor large image (primitive theme)
		KarmaGuiRenderer::AddImageTexture("../Resources/Textures/Measures.png", "Primitive Background");

		// 3. Icons Packa
		KarmaGuiRenderer::AddImageTexture("../Resources/Textures/EditorIcons/File.png", "File icon");
		KarmaGuiRenderer::AddImageTexture("../Resources/Textures/EditorIcons/Folder.png", "Folder icon");
		KarmaGuiRenderer::AddImageTexture("../Resources/Textures/EditorIcons/OpenFolder.png", "Opened Folder incon");
		KarmaGuiRenderer::AddImageTexture("../Resources/Textures/EditorIcons/UpArrow.png", "Up Arrow icon");
		KarmaGuiRenderer::AddImageTexture("../Resources/Textures/EditorIcons/RightArrow.png", "Right Attow icon");
	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(float deltaTime)
	{
		InputPolling(deltaTime);
	}

	// Rename to KarmaGuiRender for uniformity
	void EditorLayer::ImGuiRender(float deltaTime)
	{
		KGGuiID dockspaceID;
		// 1. Show the big demo window. For debug purpose!!
		static bool show = true;
		//KarmaGui::ShowDemoWindow(&show);

		// 2. A UI canvas, if I may, for the main window!!
		{
			static KarmaGuiDockNodeFlags dockspaceFlags = KGGuiDockNodeFlags_None;
			KarmaGuiWindowFlags windowFlags = KGGuiDockNodeFlags_None;

			const KarmaGuiViewport* viewport = KarmaGui::GetMainViewport();
			KarmaGui::SetNextWindowPos(viewport->WorkPos);
			KarmaGui::SetNextWindowSize(viewport->WorkSize);
			KarmaGui::SetNextWindowViewport(viewport->ID);

			// No clue
			KarmaGui::PushStyleVar(KGGuiStyleVar_WindowRounding, 0.0f);
			KarmaGui::PushStyleVar(KGGuiStyleVar_WindowBorderSize, 0.0f);

			windowFlags |= KGGuiWindowFlags_NoTitleBar | KGGuiWindowFlags_NoCollapse | KGGuiWindowFlags_NoResize | KGGuiWindowFlags_NoMove;

			windowFlags |= KGGuiWindowFlags_NoBringToFrontOnFocus | KGGuiWindowFlags_NoNavFocus;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			// hmm
			KarmaGui::PushStyleVar(KGGuiStyleVar_WindowPadding, KGVec2(0.0f, 0.0f));

			KarmaGui::Begin("KarmaSafetyDockSpace", nullptr, windowFlags);
			KarmaGui::PopStyleVar();
			KarmaGui::PopStyleVar(2);

			dockspaceID = KarmaGui::GetID("KarmaSafetyDockSpace");
			KarmaGui::DockSpace(dockspaceID, KGVec2(0.0f, 0.0f), dockspaceFlags);

			KarmaGui::End();
		}

		// The complete UI Karma shall (ever?) need. Not counting meta morpho analytic and service toolset
		{
			static CallbacksFromEditor editorCallbacks;
			editorCallbacks.openSceneCallback = std::bind(&EditorLayer::OpenScene, this, std::placeholders::_1);

			KarmaGuiMesa::RevealMainFrame(dockspaceID, m_EditorScene, editorCallbacks);
		}
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == GLFW_MOUSE_BUTTON_1)
		{
			if (KarmaGuiMesa::m_ViewportHovered)
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
		if(e.GetKeyCode() == GLFW_KEY_T)
		{
			TentativeTrigger();
		}
		else if (e.GetKeyCode() == GLFW_KEY_Y)
		{
			IterateActors();
		}

		return false;
	}

	void EditorLayer::InputPolling(float deltaTime)
	{
		if(KarmaGuiMesa::m_ViewportFocused)
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

		if (KarmaGuiMesa::m_ViewportHovered)
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

		if (Input::IsKeyPressed(GLFW_KEY_G))
		{
			//KR_CORE_INFO("someUObject name is {0}", UObject::StaticClass<UClass>()->GetPName());
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

	void EditorLayer::TentativeTrigger()
	{
		KR_INFO("========= Trigger invoked =========");

		// A test world
		if (testWorld == nullptr)
		{
			testWorld = GEngine->GetCurrentGameInstance()->GetWorldContext()->World();
			if(testWorld == nullptr)
			{
				KR_CORE_INFO("Couldnt find the template world");
			}
		}


		UClass* testActorClass = AActor::StaticClass();
		FTransform testTransform = FTransform::m_Identity;

		FActorSpawnParameters testSParameters;
		testSParameters.m_Owner = nullptr;
		testSParameters.m_Name = "TestActor" + std::to_string(m_ActorCounter++);
		testSParameters.m_OverrideLevel = testWorld->GetCurrentLevel();

		KR_INFO("-----> Spawning {0}", testSParameters.m_Name);
		AActor* someActor = testWorld->SpawnActor(testActorClass, &testTransform, testSParameters);

		// Shouldn't we be using Shivasomething?
		// delete testWorld;
	}

	void EditorLayer::IterateActors()
	{
		// Iterate over all actors, can also supply a different derived class
		// ActorItr(testWorld, AWorldSettings::StaticClass())
		for (TActorIterator<AActor> ActorItr(testWorld); ActorItr; ++ActorItr)
		{
			// print name
			//KR_INFO("Iterating over actor: {0}", (*ActorItr)->GetName());
		}

		for(FRawObjectIterator ObjectItr; ObjectItr; ++ObjectItr)
		{
			KR_INFO("Iterating over UObject: {0} with Class: {1}", (*ObjectItr)->m_Object->GetName(), (*ObjectItr)->m_Object->GetClass()->GetName());
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
