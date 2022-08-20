#pragma once

#include "krpch.h"

#include "Karma/Events/Event.h"

namespace Karma
{
	class KARMA_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnImGuiRender() = 0;
		
		virtual void OnEvent(Event& event)
		{
		}

		inline const std::string& GetName() const
		{
			return m_DebugName;
		}

	protected:
		std::string m_DebugName;
	};
}
