#pragma once

#include "Karma/Events/ApplicationEvent.h"

namespace Karma
{
	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual bool OnWindowResize(WindowResizeEvent& event) = 0;

		virtual ~GraphicsContext()
		{
		}
	};
}