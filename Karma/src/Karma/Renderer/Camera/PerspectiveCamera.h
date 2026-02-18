/**
 * @file PerspectiveCamera.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains PerspectiveCamera class
 * @version 1.0
 * @date Jun 2, 2021
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "Camera.h"

namespace Karma
{
	/**
	 * @brief Class for PerspectiveCamera, meaning Camera with depth scaling
	 */
	class KARMA_API PerspectiveCamera : public Camera
	{
	public:
		/**
		 * @brief Constructor for PerspectiveCamera class
		 *
		 * @param fovRad					field of view in Radians
		 * @param aspectRatio				aspect ration width / height (?)
		 * @param nearPlane					z distance of near plane
		 * @param farPlane					z distance of far plane
		 *
		 * @since Karma 1.0.0
		 */
		PerspectiveCamera(float fovRad, float aspectRatio, float nearPlane, float farPlane);
	};
}
