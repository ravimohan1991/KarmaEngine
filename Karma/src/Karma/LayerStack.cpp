/*
* I am little bit unsure of the implementation of PopLayer and
* PopOverlay. They basically remove the pointer to the layers/overlaya
* with doesn't deallocate the memory. The instances of layers are still there
* In destructor, now those pointers were no longer there, that memory will never
* be freed. Isn't this a memory leak.
*
* The Cherno talk about this stuff here https://youtu.be/_Kj6BSfM6P4?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&t=717
* but the above issue doesn't seem to get resolved.
*/

#include "LayerStack.h"

#include <algorithm>

namespace Karma
{
	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		for (auto layer : m_Layers)
		{
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		// Extends container by inserting new element at position.
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		// Inserts a new element at the end of the vector, right after its 
		// current last element. This new element is constructed in place 
		// using args as the arguments for its constructor.
		m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
		}
	}
}
