#pragma once

#include "Karma.h"
#include "Karma/EntryPoint.h"

namespace Karma
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void KarmaGuiRender(float deltaTime) override;

		void OpenScene(const std::string& objFileName);
		virtual void OnEvent(Event& event) override;
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);
		void InputPolling(float deltaTime);

		void TentativeTrigger();
		void IterateActors();

	private:
		std::shared_ptr<Karma::Shader> m_ModelShader;
		std::shared_ptr<Karma::VertexArray> m_ModelVertexArray;
		std::shared_ptr<Karma::Material> m_ModelMaterial;
		std::shared_ptr<Karma::Texture> m_ModelTexture;

		std::shared_ptr<Karma::PerspectiveCamera> m_EditorCamera;
		std::shared_ptr<Karma::Scene> m_EditorScene;

		float cameraTranslationSpeed = 1.0f;
		float cameraRotationSpeed = 80.0f;

		// Tentative stuff
		UWorld* testWorld;
		uint32_t m_ActorCounter;
	};
}
