#pragma once

#include "krpch.h"

#include "Karma.h"


namespace Karma
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void ImGuiRender(float deltaTime) override;

		void OpenScene();

	private:
		std::shared_ptr<Karma::Shader> m_ModelShader;
		std::shared_ptr<Karma::VertexArray> m_ModelVertexArray;
		std::shared_ptr<Karma::Material> m_ModelMaterial;
		std::shared_ptr<Karma::Texture> m_ModelTexture;
		
		std::shared_ptr<Karma::PerspectiveCamera> m_EditorCamera;
		std::shared_ptr<Karma::Scene> m_EditorScene;

		float cameraTranslationSpeed = 1.0f;
		float cameraRotationSpeed = 80.0f;
		
	};
}
