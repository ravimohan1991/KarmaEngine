/**
 * @file Layer.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the Layer class
 * @version 1.0
 * @date December 4, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Events/Event.h"
#include "Renderer/Scene.h"

namespace Karma
{
	/**
	 * @brief Base class of all Karma's Layers. For instance KarmaGuiLayer.
	 */
	class KARMA_API Layer
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		Layer(const std::string& name = "Layer");

		/**
		 * @brief A destructor
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~Layer();

		/**
		 * @brief A function called on Layer when the Layer is pushed or OverLay is pushed.
		 *
		 * @see Application::PushLayer, Application::PushOverlay
		 * @since Karma 1.0.0
		 */
		virtual void OnAttach() = 0;

		/**
		 * @brief A function called on Layer when the LayerStack is destroyed
		 *
		 * @see LayerStack::~LayerStack()
		 * @since Karma 1.0.0
		 */
		virtual void OnDetach() = 0;

		/**
		 * @brief Function called in each game loop
		 *
		 * @param deltaTime						The time step (or interval) gap between two consecutive calls (previous single loop iteration time)
		 *
		 * @see Application::Run()
		 * @since Karma 1.0.0
		 */
		virtual void OnUpdate(float deltaTime) = 0;

		// If we want KarmaGui to render the scene
		/**
		 * @brief Function called in each game loop, specifically for rendering purposes
		 *
		 * This is part of KarmaGui rendering sequence, thus different from OnUpdate().
		 *
		 * @see Application::Run()
		 * @since Karma 1.0.0
		 */
		virtual void KarmaGuiRender(float deltaTime) = 0;

		/**
		 * @brief For dispatching Events specific to the Layer
		 *
		 * @note GLFW calls the Application::OnEvent which further calls the OnEvent function in the subsequent layers and,
		 * depending on m_Handled, the depth of the call chain (measured in the sense of for-loop, upon the ordered layers) is determined.
		 * @since Karma 1.0.0
		 */
		virtual void OnEvent(Event& event)
		{
		}

		/**
		 * @brief Getter for the name of the Layer (debugging purposes).
		 *
		 * @since Karma 1.0.0
		 */
		inline const std::string& GetName() const
		{
			return m_DebugName;
		}

	protected:
		std::string m_DebugName;
	};
}
