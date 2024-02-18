/**
 * @file LayerStack.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the LayerStack class
 * @version 1.0
 * @date December 4, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Layer.h"

namespace Karma
{
	/**
	 * @brief Class concerned with the stacked Layer (what a way to frame the idea, if I may say so)
	 */
	class KARMA_API LayerStack
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		LayerStack();

		/**
		 * @brief Destructor for freeing up the Layer occupied resource and calling Layer::OnDetach
		 * function in each Layer.
		 *
		 * @since Karma 1.0.0
		 */
		~LayerStack();

		/**
		 * @brief LayerStack::m_Layers.emplace() is used to place the supplied layer at an index in ascending order.
		 * For instance the layer inserted first has least index and subsequent layers have higher indices ordered accordingly.
		 *
		 * @note The natural way to PushLayer is by the API in Karma's Application class, Application::PushLayer which, in turn, calls
		 * this function.
		 *
		 * @param layer						The Layer to be placed
		 * @since Karma 1.0.0
		 */
		void PushLayer(Layer* layer);

		/**
		 * @brief Places the Layer at the front most position, in the sense, after the last inserted layer.
		 *
		 * @note The natural way to PushOverly is by the API in Karma's Application class, Application::PushOverlay which, in turn, calls
		 * this function.
		 *
		 * @param layer						The Layer to be pushed as overlay
		 * @since Karma 1.0.0
		 */
		void PushOverlay(Layer* overlay);

		/**
		 * @brief For popping a Layer in the LayerStack and doing necessary arrangements (Layer counter decrement and calling OnDetach function)
		 *
		 * @param layer						The layer to be popped
		 * @since Karma 1.0.0
		 */
		void PopLayer(Layer* layer);

		/**
		 * @brief For popping the specified Overlay and doing necessary arrangements (calling OnDetach function)
		 *
		 * @param layer						The overlay layer to be popped
		 * @since Karma 1.0.0
		 */
		void PopOverlay(Layer* layer);

		/**
		 * @brief The Layer stack iterator with first element of the LayerStack
		 *
		 * @see Application::Run
		 * @since Karma 1.0.0
		 */
		std::vector<Layer*>::iterator begin()
		{
			return m_Layers.begin();
		}

		/**
		 * @brief The Layer stack iterator with the last element (?) of the LayerStack
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<Layer*>::iterator end()
		{
			return m_Layers.end();
		}

	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};
}
