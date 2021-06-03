#pragma once

#include "Camera.h"

namespace Karma
{
	class KARMA_API PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(float fovRad, float aspectRatio, float nearPlane, float farPlane);
	};
}