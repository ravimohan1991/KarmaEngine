#pragma once

#include "Camera.h"

namespace Karma
{
	class KARMA_API OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);
	};

}