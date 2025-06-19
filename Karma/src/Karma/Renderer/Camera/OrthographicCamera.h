/**
 * @file OrthographicCamera.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains OrthographicCamera class
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
	 * @brief Class for OrthographicCamera meaning Camera without depth scaling
	 */
	class KARMA_API OrthographicCamera : public Camera
	{
	public:
		/**
		 * @brief Default constructor
		 *
		 * @param left					leftmost x coordinate in world units
		 * @param right					rightmost x coordinate in world units
		 * @param bottom				bottom most y coordinate in world units
		 * @param top					top most y coordinate in world units
		 *
		 * @since Karma 1.0.0
		 */
		OrthographicCamera(float left, float right, float bottom, float top);
	};

}
