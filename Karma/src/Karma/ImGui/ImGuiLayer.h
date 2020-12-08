#pragma once

#include "Karma/Layer.h"
#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"

namespace Karma
{
	class KARMA_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& event) override;

	private:
		float m_Time = 0.0f;
	};
}