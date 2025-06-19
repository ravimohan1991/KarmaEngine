/**
 * @file Camera.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains Camera class
 * @version 1.0
 * @date December 28, 2020
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "glm/glm.hpp"

namespace Karma
{
	/**
	 * @brief Base class of Camera
	 *
	 * @todo This class needs major revamp with reparenting(?) with AActor, just like Unreal Engine's CameraActor
	 */
	class KARMA_API Camera
	{
	public:
		/**
		 * @brief Default constructor
		 *
		 * @param initialCameraPosition					Position to spawn camera at
		 * @since Karma 1.0.0
		 */
		Camera(const glm::vec3& initialCameraPosition);

		/**
		 * @brief Destructor
		 *
		 * @since Karma 1.0.0
		 */
		virtual ~Camera();

		/**
		 * @brief Getter for current position of the Camers
		 *
		 * @return glm::vec3 m_Position
		 * @todo Use different data structure, provided in Ganit/Transform.h
		 *
		 * @since Karma 1.0.0
		 */
		const glm::vec3& GetPosition() const { return m_Position; }

		/**
		 * @brief Getter for current rotation
		 *
		 * @return float m_Rotation
		 * @since Karma 1.0.0
		 */
		float GetRotation() const { return m_Rotation; }

		/**
		 * @brief Differentiates between single button press and drag.
		 *
		 * If drag is detected then the relevant orientation of the Camera is computed and applied,
		 * by setting m_Pitch and m_Roll, else the status m_IsLeftMouseButtonPressed is set.
		 *
		 * @see KarmaAppInputPolling
		 * @since Karma 1.0.0
		 */
		void LeftMouseButtonPressed();

		/**
		 * @brief Setting m_IsLeftMouseButtonPressed to false
		 *
		 * @since Karma 1.0.0
		 */
		void LeftMouseButtonReleased();

		/**
		 * @brief Computes the pointing vector of the camera (outward positive) given the
		 * current pitch, and roll. (no yaw)
		 *
		 * @since Karma 1.0.0
		 */
		void ComputeCameraFront();

		/**
		 * @brief Sets the current position of the Camera
		 *
		 * @param position						The position to set to
		 * @since Karma 1.0.0
		 */
		void SetPosition(const glm::vec3& position);

		/**
		 * @brief Sets the current rotation of the Camera
		 *
		 * @param rotation				The rotation to be se
		 *
		 * @deprecated m_Rotation is no longer used for the orientation. See m_Pitch and m_Roll.
		 *
		 * @since Karma 1.0.0
		 */
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		/**
		 * @brief Set the front vector of Camera
		 *
		 * @param front					The value to set the front vector to
		 * @since Karma 1.0.0
		 */
		inline void SetCameraFront(const glm::vec3& front) { m_CameraFront = front; }

		/**
		 * @brief Set the up vector of Camera
		 *
		 * @param up					The value to set the up vector to
		 * @since Karma 1.0.0
		 */
		inline void SetCameraUp(const glm::vec3& up) { m_CameraUp = up; }

		/**
		 * @brief Translate in the direction of front vector
		 *
		 * @param amount				The magnitude of the  translation in front direction
		 * @since Karma 1.0.0
		 */
		void MoveForward(float amount);

		/**
		 * @brief Move sideways in the direction of cross product of front vector and up vector
		 *
		 * @param amount				The magnitude of the translation in the side direction
		 * @since Karma 1.0.0
		 */
		void MoveSideways(float amount);

		/**
		 * @brief Move in the direction of Up
		 *
		 * @param amount				The magnitude of translation
		 * @since Karma 1.0.0
		 */
		void MoveUp(float amount);

		/**
		 * @brief Rotate about Y-axis
		 *
		 * @param amount				The magnitude of the angle of rotation
		 * @since Karma 1.0.0
		 */
		void RotateAboutYAxis(float amount);

		/**
		 * @brief Rotate about X-axis
		 *
		 * @param amount				The magnitude of the angle of rotation
		 * @since Karma 1.0.0
		 */
		void RotateAboutXAxis(float amount);

		/**
		 * @brief Getter for ProjectionMatrix
		 *
		 * @since Karma 1.0.0
		 */
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

		/**
		 * @brief Getter for ViewMatrix
		 *
		 * @since Karma 1.0.0
		 */
		const glm::mat4& GetViewMatirx() const { return m_ViewMatrix; }

	private:
		void RecalculateViewMatrix();

	protected:
		void InitializePitchRoll();

	protected:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		float m_Rotation = 0.0f;

		bool m_IsLeftMouseButtonPressed = false;
		float m_LastMouseX, m_LastMouseY;

		const float m_Sensitivity = 0.1f;
		float m_Roll = 0.0f;
		float m_Pitch = 0.0f;
	};
}
